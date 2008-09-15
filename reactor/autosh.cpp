#include <stdexcept>
#include <string>

#include <iostream>
#include <sstream>

#include "tinfra/cmd.h"
#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"
#include "tinfra/symbol.h"

#include "aio.h"
//#include "tinfra/aio.h"

#include "protocols.h"

using tinfra::aio::Dispatcher;
using tinfra::aio::Channel;

#define TINFRA_DECLARE_STRUCT template <typename F> void apply(F& field) const
#define FIELD(a) field(S::a, a)

namespace AutoSH {
    namespace S {
        extern tinfra::symbol message_type;
        extern tinfra::symbol message_length;
        
        extern tinfra::symbol request_id;
        extern tinfra::symbol command;
        extern tinfra::symbol environment;
        
        extern tinfra::symbol task_id;
        extern tinfra::symbol stream_id;
        extern tinfra::symbol what;
        extern tinfra::symbol status;
        extern tinfra::symbol data;
        
        extern tinfra::symbol description;
    }
    
    struct MessageHeader {        
        int           message_type;
        size_t        message_length;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(message_type);
            FIELD(message_length);
        }
    };

    struct InvokeRequest {
        unsigned long request_id;
        
        std::vector<std::string> command;
        std::vector<std::string> environment;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(request_id);
            FIELD(command);
            FIELD(environment);
        }
    };
    
    struct InvokeResult {
        unsigned long request_id;
        int           status;
        std::string   description;
        
        int           task_id;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(task_id);
            FIELD(status);
            FIELD(description);
            FIELD(channel_id);
        }
    };
    
    struct ChannelEvent {
        int           task_id;
        int           stream_id;
        int           what;
        int           status;
        std::string   data;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(task_id);
            FIELD(stream_id);
            FIELD(what);
            FIELD(status);
            FIELD(data);
        }
    };
    
    struct NoResponse {
    };
}

namespace AutoSH { namespace S {
    tinfra::symbol message_type("message_type");
    tinfra::symbol message_length("message_length");
    
    tinfra::symbol message_id("request_id");
    tinfra::symbol command("command");
    tinfra::symbol environment("environment");
    
    tinfra::symbol task_id("task_id");
    tinfra::symbol stream_id("stream_id");
    tinfra::symbol status("status");
    tinfra::symbol what("what");
    tinfra::symbol data("data");
    
    tinfra::symbol description("description");
} } // end namespace AutoSH::S

namespace AutoSH {

class NetworkProtocolHandler: public ::ProtocolHandler {
public:
    enum {
        READING_MESSAGE_HEADER,
        READING_MESSAGE_CONTENT,
        FINISHED
    } state;
    
    NetworkProtocolHandler()
        : state(READING_MESSAGE_HEADER) 
    {
    }
    
    MessageHeader received_message_header;
    virtual int  accept_bytes(const char* data, int length, tinfra::io::stream* channel)
    {        
        switch( state )  {
        case READING_MESSAGE_HEADER:
            return 0;
        case READING_MESSAGE_CONTENT:
            return 0;
        case FINISHED:
            // TODO: don't process any data after protocol closing
            return length;
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
        return false;
    }
};

class Processor {
    Dispatcher&             io_dispatcher;
    NetworkProtocolHandler& network_protocol;
    
    tinfra::cmd::app&   app;
    tinfra::io::stream* sink;
    
    int last_task_id;
    
    typedef tinfra::aio::StreamChannel StreamChannel;
    typedef tinfra::subprocess subprocess;
    
    struct Task {
        int id;
        tinfra::subprocess* process;
        
        StreamChannel channel_in;
        StreamChannel channel_out
        StreamChannel channel_err;
        StreamChannel channels[3];
        int           channel_count;
        Task(int id, subprocess* pprocess):
            id(id),
            process(pprocess),
            channel_in(pprocess.get_stdin(), false),
            channel_out(pprocess.get_stdout(), false),
            channel_err(pprocess.get_stderr(), false))
        {
            channel_count = 3;
            channels[0] = &channel_in;
            channels[1] = &channel_out;
            channels[2] = &channel_err;
        }
    };
public:
    
    Processor(Dispatcher&, NetworkProtocolHandler&);
        
    // called when there is invoke_request received from peer
    void invoke_request(InvokeRequest const& request);
    
    // called when there is channel event received from peer
    void channel_event(ChannelEvent const& event);
    
    // called when there is data from task readed in some buffer
    void task_data(int task_id, int stream_id, 
                   const char* data, size_t length);
        
    int get_next_task_id();
    
    
    // send message to peer
    template <typename T>
    void send_reply(int type, T const&t);
    
};

Processor::Processor(Dispatcher& d, NetworkProtocolHandler& np):
    io_dispatcher(d),
    network_protocol(np),
    app(tinfra::cmd::app::get()),
    last_task_id(0)
{
}

int Processor::get_next_task_id()
{
    return ++last_task_id;
}

void Processor::invoke_request(InvokeRequest const& request)
{
    std::auto_ptr<subprocess> sp(tinfra::subprocess::create());
    
    sp->set_stdout_mode(tinfra::subprocess::REDIRECT);
    sp->set_stderr_mode(tinfra::subprocess::REDIRECT);
    sp->set_stderr_mode(tinfra::subprocess::REDIRECT);
    
    InvokeResult result;
    try {
        sp->start(request.command);
        Task* new_task = new Task(get_next_task_id(), sp.release());
        
        io_dispatcher.add_channel(new_task->channel_in);
        
        io_dispatcher.add_channel(new_task->channel_out);
        io_dispatcher.listen_channel(new_task->channel_out, Dispatcher::READ, true);
        
        io_dispatcher.add_channel(new_task->channel_err);
        io_dispatcher.listen_channel(new_task->channel_err, Dispatcher::READ, true);
        
        // TODO: add task to pool of active tasks
        
        // fill in success reply
        result.request_id = request.result_id;
        result.status = 1; // RUNNING;
        result.description = "";
        result.task_id = new_task->id;
    } catch( std::exception const& e) {
        std::string error_text = e.what(); 
        app.error(tinfra::fmt("invoke_request failed: %s" % error_text);
        
        // fill in error reply
        result.request_id = request.result_id;
        result.status = 0; // FAILED;
        result.description = error_text;
        result.task_id = 0;
    }
    send_reply(2 /* INVOKE_REPLY */, result);
}
    
void Processor::channel_event(ChannelEvent const& event)
{
    Task* task = get_task(event.task_id);
    if( task == 0 ) {
        app.warning(fmt("bad task=%i") % event.task_id );
        // TODO close this channel ?
        //    discussion: yes. it's an abuse
        //                no. because these could be delayed requests
        //                from queueing client
        return
    }
    
    if( event.stream_id < 0 || event.stream_id >= task->channel_count ) {
        app.warning(fmt("task=%i bad stream_id=%i") % event.task_id % event.stream_id);
        
        return;
    }
    
    // TODO: deliver bytes to channel in non-blocking way
    StreamChannel* channel = task->channels[event.task_id];
    
    try {
        channel->get_stream()->write(event.data.str(), event.data().size());
    } catch( std::exception const& e) {
        std::string error_text = e.what(); 
        app.error(tinfra::fmt("data forward to process failed: %s" % error_text);
        // abort_task(event.task_id)
    }
}

void Processor::task_data(int task_id, int stream_id, const char* data, size_t length)
{
    ChannelEvent message;
    message.task_id = task_id;
    message.stream_id = stream_id;
    message.data.assign(data, length);
    
    send_reply(3 /* CHANNEL_EVENT */, message);
}

template <typename T>
void Processor::send(int type, T const&t)
{
    app.error(tinfra::fmt("unable to send message type=%i, not implemented") % type);
    std::ostringstream builder_buffer;
    // TODO build message
    
    np->
}

class ServerChannel: public tinfra::aio::ListeningChannel {
public:
    ServerChannel(int port = 10022): ListeningChannel(port) {}
protected: 
    virtual void on_accept(Dispatcher& d, tinfra::io::stream* stream, std::string const& remote_peer)
    {
        ::ProtocolWrapperChannel* protocol = new ::ProtocolWrapperChannel(stream, new NetworkProtocolHandler());
        
        d.add_channel(protocol);
        d.listen_channel(protocol, Dispatcher::READ, true);
    }
};

} // end namespace AutoSH

tinfra::cmd::app* app;

using std::string;
using std::vector;

int serve()
{
    std::auto_ptr<Dispatcher> ctx = tinfra::aio::createDispatcher();
    
    AutoSH::ServerChannel autosh_service;
    
    ctx->add_channel( & autosh_service ) ;
    ctx->listen_channel( & autosh_service, Dispatcher::READ, true);
    
    while( true ) {
        ctx->step();
    }
    return 0;
}

typedef vector<string> string_list;

int invoke(string const& address, string_list const& command)
{
    app->fail("invocation not implemented");
    return 0;
}

int autosh_main(int argc, char** argv)
{
    app = &tinfra::cmd::app::get();
    if( argc == 2 && std::strcmp(argv[1], "serve") == 0 )
        return serve();
    
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

TINFRA_MAIN(autosh_main);

