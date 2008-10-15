//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <stdexcept>
#include <string>

#include <iostream>
#include <sstream>

#include "tinfra/cmd.h"
#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"
#include "tinfra/symbol.h"

#include "tinfra/subprocess.h"
#include "tinfra/fmt.h"
#include "aio.h"
//#include "tinfra/aio.h"


#include "protocols.h"

using tinfra::aio::Dispatcher;
using tinfra::aio::Channel;

#define TINFRA_DECLARE_STRUCT template <typename F> void apply(F& field) const
#define FIELD(a) field(S::a, a)

namespace remote_shell {
    namespace S {
        extern tinfra::symbol command;
        extern tinfra::symbol environment;
        
        extern tinfra::symbol stream_id;
        extern tinfra::symbol what;
        extern tinfra::symbol status;
        extern tinfra::symbol data;
    }
    
    enum status_code {
        OK = 0,
        FAILED = 1,
        FINISHED = 2
    };
    
    struct request {
        std::vector<std::string> command;
        std::vector<std::string> environment;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(command);
            FIELD(environment);
        }
    };
    
    struct stream_data {
        int           stream_id;        
        int           status;
        int           what;
        std::string   data;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(stream_id);
            FIELD(what);
            FIELD(status);
            FIELD(data);
        }
    };
}

#include "tinfra/tinfra.h"
#include "raw_net_serializer.h"
#include "message_raw.h"

namespace remote_shell { namespace S {
    tinfra::symbol command("command");
    tinfra::symbol environment("environment");
    
    tinfra::symbol stream_id("stream_id");
    tinfra::symbol status("status");
    tinfra::symbol what("what");
    tinfra::symbol data("data");
} } // end namespace remote_shell::S

using tinfra::fmt;
using tinfra::subprocess;
using tinfra::io::stream;
using std::auto_ptr;
using std::string;

static void initialize_async_file(int file)
{
    tinfra::io::socket::set_blocking(file, false);
}

namespace remote_shell {

class ServiceHandler: public message_raw::ProtocolHandler {
public:
    enum {
        WAITING_REQUEST,
        PROCESSING,
        FINISHED
    } state;
    
    NetworkProtocolHandler()
        : state(WAITING_REQUEST) 
    {
    }
    
    stream* peer_channel;
    void set_peer_channel(stream* peer_channel)
    {
        this->peer_channel = peer_channel;
    }
    void accept_message(const string& message, stream* feedback_channel)
    {
        set_peer_channel(feedback_channel);
        
        tinfra::raw_net_serializer::reader reader(message.data(), message.size());
        
        if( state == WAITING_REQUEST ) {
            request incoming_request;
            tinfra::mutate(incoming_request, reader);
            process_request(incoming_request, feedback_channel);
        } else {
            fail_and_disconnect(FAILED, EINVAL, feedback_channel);
        }
    }
    
    auto_ptr<subprocess> process;
    bool process_is_running;
    
    void process_request(incoming_request const& ir, stream* feedback_channel)
    {
        process = auto_ptr<subprocess>(subprocess::create());
        
        process->set_stdout_mode(tinfra::subprocess::REDIRECT);
        process->set_stderr_mode(tinfra::subprocess::REDIRECT);
        process->set_stderr_mode(tinfra::subprocess::REDIRECT);
        
        
        // TODO: support environment
        try {
            sp->start(ir.command);
            
            process_is_running = true;
            
            stream_data response;
            
            response.stream_id = -1;
            response.status = OK;
            response.what = 0;
            // response.data = empty;
            
            initialize_async_file(sp->get_stdin()->native());
            initialize_async_file(sp->get_stdout()->native());
            initialize_async_file(sp->get_stderr()->native());
            
            aio.put(sp->get_stdin(), 0                , this);
            aio.put(sp->get_stdout(), Dispatcher::READ, this);
            aio.put(sp->get_stderr(), Dispatcher::READ, this);
            
            send_messsage(feedback_channel, tinfra::raw_net_serializer::serialize(response));
        } catch( std::exception& e) {
            // TODO log an error
            disconnect(FAILED, EINVAL, feedback_channel)
        }
    }
    
    void disconnect(status_code sc, int what, stream* feedback_channel)
    {
        ensure_process_killed();
        
        stream_data response;
        response.stream_id = -1;
        response.status = sc;
        response.what = what;
        
        send_messsage(feedback_channel, tinfra::raw_net_serializer::serialize(response));
        
        feedback_channel->close();
        state = FINISHED;
    }
    
    void ensure_process_killed()
    {
        if( process.get() == 0 ) {
            return;
        }
        
        //
        sp->terminate();
        
        wait_cleanly_for_process();
    }
    
    int wait_cleanly_for_process()
    {
        aio.remove(sp->get_stdin());
        aio.remove(sp->get_stdout());
        aio.remove(sp->get_stderr());
        
        cp->get_stdin()->close();
        cp->get_stdout()->close();
        sp->wait();
        return sp->get_exit_code();
    }
    
    // those two are events generated by subprocess
    virtual void event(Dispatcher& d, Channel c, int event)
    {
        if( c == sp->get_stdout()) {
            stream_id = 1;
        } else if( c == sp->get_stdin()) {
            stream_id = 0;            
        } else if( c == sp->get_stderr()) {
            stream_id = 2;
        } else {
            // TODO: log error
            //       event on unknown channel
            abort();
        }
        
        if( stream_id == 0 ) {
            // try buffered input data to process
        } else {
            stream_data response;
            response.stream_id = stream_id;
            response.status = OK;
            response.what = 0;
            char buf[1024];
            int n = c->read(buf, 1024);
            response.data.assign(buf, n);
            
            send_messsage(peer_channel, tinfra::raw_net_serializer::serialize(response));
        }
    }
    
    virtual void failure(Dispatcher& d, Channel c, int error)
    {
        // TODO here we should check if this was close
        //      due to clean exit of parent process
        //      clean:
        //            send FINISHED, exit_code
        //      not clean
        //            send FAILED, ???
        
        int exit_code = wait_cleanly_for_process();
        if( exit_code == -1 ) {
            ensure_process_killed();
            disconnect(FAILED, -1, peer_channel);
        } else {
            disconnect(FINISHED, exit_code, peer_channel);
        }
        
    }
    
    
    virtual void write_completed(size_t bytes_sent, size_t bytes_queued)
    {
        
    }
    
    virtual void eof(int direction)
    {
    }

    /// check if this protocol handler has finished reading
    virtual bool is_finished()
    {
        return state == FINISHED;
    }
};

class ClientHandler: public message_raw::ProtocolHandler {
    enum {
        INITIALIZATION,
        PROCESSING,
        FINISHED
    } state;
    
    /// Input from process.
    ///
    /// Called by autosh protocol handler when client receives valid data
    /// from remote process.
    /// 
    /// After successfull spawning of remote process there is one signal with
    /// stream=-1 and empty data. You can start client-oriented protocols
    /// and/or remember feedback channel object for later use.
    ///
    ///   @param stream    id of stream (1 stdout, 2 stderr)
    ///   @param input     subject data
    ///   @param feedback  channel to which you should send feedback
    virtual void accept_input(int stream, const string& input, stream* feedback) = 0;
    
    /// Communication finished.
    ///
    /// Called by protocol handler when communication has been finished for any reason:
    ///    exit_code >= 0  - it's an clean exit and exit_code means remote process exit code
    ///    exit_code == -1 - communication error - link failed and dropped
    virtual void finished(int exit_code) = 0;
    
    void accept_message(const string& message, stream* feedback_channel)
    {
        set_peer_channel(feedback_channel);
        
        tinfra::raw_net_serializer::reader reader(message.data(), message.size());
        
        stream_data sd;
        
        tinfra::mutate(sd, reader);
        
        switch( sd.status ) {
        case OK:
            accept_input(sd.stream_id, sd.data, feedback);
            break;
        case FINISHED:
            finished(sd.what);
            state = FINISHED;
            break;
        case FAILED:
            finished(-1);
            state = FINISHED;
        }
    }
    
    virtual void failure(Dispatcher& d, Channel c, int error)
    {
        // TODO here we should check if this was close
        //      due to clean exit of parent process
        //      clean:
        //            send FINISHED, exit_code
        //      not clean
        //            send FAILED, ???
        
        finished(-1);
        state = FINISHED;
    }
    
    virtual void write_completed(size_t bytes_sent, size_t bytes_queued)
    {
    }
    
    virtual void eof(int direction)
    {
        finished(-1);
        state = FINISHED;
    }

    /// check if this protocol handler has finished reading
    virtual bool is_finished()
    {
        return state == FINISHED;
    }
};

} // end namespace remote_shell

using std::string;
using std::vector;

int serve()
{
    auto_ptr<Dispatcher> ctx = tinfra::aio::create_network_dispatcher();
    
    auto_ptr<stream> in  = tinfra::io::open_native(0);
    auto_ptr<stream> out = tinfra::io::open_native(1);
    
    initialize_async_file(0);
    initialize_async_file(1);
    
    remote_shell::ServiceHandler service;
    
    //ProtocolListener should renamed to protocol_aio_adaptor
    ProtocolListener adaptor(in.get(), out.get(), &service);    
    ctx->put( in, Dispatcher::READ, &adaptor ) ;
    ctx->put( in, 0               , &adaptor ) ;
    
    while( ! service.is_finished() ) {        
        ctx->step();
        tinfra::test_interrupt();
    }
    
    return 0;
}

int listen(int port)
{
    auto_ptr<Dispatcher> ctx = tinfra::aio::create_network_dispatcher();

    initialize_async_file(0);
    initialize_async_file(1);
    
    remote_shell::ServiceHandler service;
    
    //ProtocolListener should renamed to protocol_aio_adaptor
    ProtocolListener adaptor(in.get(), out.get(), &service);    
    ctx->put( in, Dispatcher::READ, &adaptor ) ;
    
    while( ! service.is_finished() ) {
        ctx->step();
        
        tinfra::test_interrupt();
    }
}

typedef vector<string> string_list;

class stdout_autosh_client_handler: public remote_shell::ClientHandler {
};

int invoke(string const& address, string_list const& command)
{
    auto_ptr<Dispatcher> ctx = tinfra::aio::create_network_dispatcher();
    
    stdout_autosh_client_handler client;
    
    ctx->create(Dispatcher::CLIENT, address, &client, Dispatcher::READ);
    
    client.invoke("uname -a");
    
    while( !client.is_finished() ) {
        ctx->step();
        
        tinfra::test_interrupt();
    }
    
    return 0;
}

int autosh_main(int argc, char** argv)
{
    if( argc == 2 && std::strcmp(argv[1], "serve") == 0 ) {
        return serve();
    }
    
    if( argc < 3 ) {
        app->fail("bad usage");
        return 1;
    }
    
    string address = argv[1];
    string_list command;
    for( int i = 2; i < argc; ++i )
        command.push_back(argv[i]);
    return invoke(address, command);
}

TINFRA_MAIN(autosh_main)


