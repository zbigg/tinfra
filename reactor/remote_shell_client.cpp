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

#include "tinfra/tinfra.h"
#include "raw_net_serializer.h"

#include "message_raw.h"

#include "remote_shell.h"
#include "remote_shell_protocol.h"

namespace remote_shell {

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
