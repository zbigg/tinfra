//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_protocol_buffer_h__
#define __tinfra_protocol_buffer_h__

#include <stdexcept>
#include <string>

#include <iostream>
#include <sstream>

#include "tinfra/cmd.h"
#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"


#include "tinfra/aio.h"

class ProtocolHandler {
public:
    virtual ~ProtocolHandler() {}
    
    /// Framework calls when IO has some data buffered
    /// Protocol should consume as much data as he can
    /// And then return number of bytes consumed    
    /// @returns 0 if that protocol is unable to assemble any message - IO must gather more data
    /// @returns > 0 length of consumed message
    virtual int  accept_bytes(const char* , int, tinfra::io::stream*) = 0;
    
    // Framework informs that it has successfully send some bytes
    // via wire.
    virtual void write_completed(size_t bytes_sent, size_t bytes_queued) = 0;

    // Framework informs that channel has reached EOF
    // direction can be set of
    //     Dispatcher::READ
    //     Dispatcher::WRITE
    // to indicate which side is closed now.
    virtual void eof(int direction) = 0;
        
    /// framework checks if this protocol has finished
    virtual bool is_finished() = 0;
};

// TODO: ProtocolListener should renamed to protocol_aio_adaptor

class lazy_protocol_listener: public tinfra::aio::Listener {
public:
    lazy_protocol_listener(tinfra::io::stream* io, ProtocolHandler* handler);
    lazy_protocol_listener(tinfra::io::stream* in, tinfra::io::stream* out, ProtocolHandler* handler);
    
    virtual ~ProtocolListener();
    
    tinfra::io::stream* get_input_stream() { return &public_stream; }
    tinfra::io::stream* get_output_stream() { return &public_stream; }
    
protected:
    // Listener contract
    virtual void failure(tinfra::aio::Dispatcher& dispatcher, tinfra::io::stream* channel, int event);
    
    virtual void event(tinfra::aio::Dispatcher& dispatcher, tinfra::io::stream* channel, int event);
    
private:
    tinfra::io::stream* in;  // input channel
    tinfra::io::stream* out; // output channel, may be the same as in
    
    ProtocolHandler* handler;
    typedef std::string buffer;

    buffer received_bytes;
    buffer to_send;

    bool   closed;          /// channel is really closed
    bool   close_requested; /// protocol has requested close
    bool   read_eof;        /// stream has reported EOF when reading
    bool   write_eof;       /// stream has reported EOF when writing 

    void close();
    
    void data_available(tinfra::aio::Dispatcher& r);
    
    void write_possible(tinfra::aio::Dispatcher& r);
    // implementation stream
    
    class BufferedNonBlockingStream: public tinfra::io::stream {
        ProtocolListener& base;
    public:
        
        BufferedNonBlockingStream(ProtocolListener& b);

        virtual intptr_t native() const;
        
        virtual void release();
        
        virtual void close();
        
        virtual int seek(int pos, seek_origin origin = start);
        
        virtual int read(char* dest, int size);
        
        virtual int write(const char* data, int size);
    
        virtual void sync();
    };
    
    BufferedNonBlockingStream public_stream;
    
    int read_next_chunk();
    
    void update_listen_status(tinfra::aio::Dispatcher& r);
};

#endif

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
