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
#include <cassert>

#include "tinfra/cmd.h"
#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"

#include "aio.h"
//#include "tinfra/aio.h"

#include "protocols.h"

using tinfra::aio::Dispatcher;
using tinfra::aio::Channel;
//
// ProtocolListener
//

ProtocolListener::ProtocolListener(tinfra::io::stream* channel, ProtocolHandler* handler)
    : channel(channel), 
      handler(handler),
      closed(false),
      close_requested(false),
      read_eof(false),
      write_eof(false),
      public_stream(*this)
{
}

ProtocolListener::~ProtocolListener()
{
    delete handler;
}

void ProtocolListener::close() {
    if( to_send.size() > 0 ) {
        close_requested = true;
    } else { 
        closed = true;
    }
}

void ProtocolListener::failure(Dispatcher& dispatcher, Channel channel, int error) { 
    delete this;
}

void ProtocolListener::event(Dispatcher& dispatcher, Channel channel, int event) {
    this->channel = channel;
    if( event == Dispatcher::READ) 
        data_available(dispatcher);
    else if( event == Dispatcher::WRITE )
        write_possible(dispatcher);
    else
        assert(false);
    
    update_listen_status(dispatcher);
}

void ProtocolListener::data_available(Dispatcher& r)
{
    // general idea
    //  - read till end of buffer
    //  - while possible consume using protocol handler
    //    
    while (! read_eof && ! handler->is_finished() ) {
        while( ! received_bytes.empty() ) {
            int accepted = handler->accept_bytes(received_bytes.data(), received_bytes.size(), get_output_stream());
            if( accepted == 0 ) 
                break; // not nough data, try read some
            received_bytes.erase(0, accepted);
        }
        
        if( read_next_chunk() <= 0 )
            break;
        // something read, retry with protocol
    }
}

int ProtocolListener::read_next_chunk()
{
    if( closed || close_requested ) 
        return 0;
    try {
        char buffer[1024];
        int readed = channel->read(buffer, sizeof(buffer));
        if( readed == 0 ) {
            handler->eof(Dispatcher::READ);
            read_eof = true;
            return 0;
        }
        received_bytes.append(buffer, readed);
        return readed;
    } catch( tinfra::io::would_block& b) {
        // nothing read so protocol can't continue
        return -1;
    }
}
    
void ProtocolListener::write_possible(Dispatcher& r)
{
    try {
        if( ! write_eof && to_send.size() > 0 ) {
            int written = channel->write(to_send.data(), to_send.size());
            if( written == 0 ) {
                handler->eof(Dispatcher::WRITE);
                write_eof = true;
                
            } else if( written > 0 ) {
                to_send.erase(0, written);
                handler->write_completed(written, to_send.size());
            }
        }
    } catch( tinfra::io::would_block&) {
        // ignore it!
    }
    if( close_requested && to_send.size() == 0 ) {
        closed = true;
    }
}
    
void ProtocolListener::update_listen_status(Dispatcher& dispatcher)
{
    if( closed ) {
        dispatcher.close(channel);
        delete this;
        return;
    }
    if( read_eof || handler->is_finished() ) {
        dispatcher.wait(channel, Dispatcher::READ, false);
    } else {
        dispatcher.wait(channel, Dispatcher::READ, true);
    }
    
    if( write_eof || to_send.size() == 0 ) {
        dispatcher.wait(channel, Dispatcher::WRITE, false);
    } else {
        dispatcher.wait(channel, Dispatcher::WRITE, true);
    }
}

//
// ProtocolListener::BufferedNonBlockingStream
//

    
ProtocolListener::BufferedNonBlockingStream::BufferedNonBlockingStream(ProtocolListener& b)
    : base(b) 
{}

intptr_t ProtocolListener::BufferedNonBlockingStream::native() const 
{
    return base.channel->native();
}

void ProtocolListener::BufferedNonBlockingStream::release()
{
    throw std::logic_error("ProtocolListener::BufferedNonBlockingStream: release() not supported");
}

void ProtocolListener::BufferedNonBlockingStream::close()
{
    base.close();
}

int ProtocolListener::BufferedNonBlockingStream::seek(int pos, seek_origin origin)
{
    throw std::logic_error("ProtocolListener::BufferedNonBlockingStream: seek() not supported");
}

int ProtocolListener::BufferedNonBlockingStream::read(char* dest, int size)
{
    int readed = 0;
    while( readed < size ) {
        if( (int)base.received_bytes.size() >= size ) {
            memcpy(dest, base.received_bytes.data(), size);
            base.received_bytes.erase(0, size);
            return size;
        } else {
            int r = base.read_next_chunk();
            if( r == 0 )
                return readed;
            if( r == -1 )
                throw tinfra::io::would_block("");
        }
    }
    return readed;
}

int ProtocolListener::BufferedNonBlockingStream::write(const char* data, int size)
{
    if( base.write_eof || base.closed || base.close_requested )
        return 0;
    int written = 0;
    if( base.to_send.size() == 0 ) {
        try {
            written = base.channel->write(data, size);
            if( written == 0 ) {
                base.write_eof = true;
                base.handler->eof(Dispatcher::WRITE);
                return 0;
            }
            base.handler->write_completed(written, size-written + base.to_send.size() );
        } catch( tinfra::io::would_block& w) {
            // ignore it, written = 0, so all will be buffered
        }
    }
    if( written < size ) {
        base.to_send.append(data + written, size - written);
        written = size;
    }
    return written;
}
void ProtocolListener::BufferedNonBlockingStream::sync() 
{
    // not supported
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
