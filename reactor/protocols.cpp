#include <stdexcept>
#include <string>

#include <iostream>
#include <sstream>

#include "tinfra/cmd.h"
#include "tinfra/io/stream.h"
#include "tinfra/io/socket.h"

#include "aio.h"
//#include "tinfra/aio.h"

#include "protocols.h"

using tinfra::aio::Dispatcher;
using tinfra::aio::Channel;
//
// ProtocolWrapperChannel
//

ProtocolWrapperChannel::ProtocolWrapperChannel(tinfra::io::stream* channel, ProtocolHandler* handler)
    : channel(channel), 
      handler(handler),
      closed(false),
      close_requested(false),
      read_eof(false),
      write_eof(false),
      public_stream(*this)
{
}

ProtocolWrapperChannel::~ProtocolWrapperChannel()
{
    delete handler;
    delete channel;
}

int  ProtocolWrapperChannel::file() { 
    return channel->native(); 
}

void ProtocolWrapperChannel::close() {
    if( to_send.size() > 0 ) {
        close_requested = true;
    } else { 
        closed = true;
        channel->close();
    }
}
void ProtocolWrapperChannel::failure(Dispatcher& r) { 
    r.remove_channel(this); 
    channel->close();
    delete this;
}

void ProtocolWrapperChannel::hangup(Dispatcher& r) { 
    r.remove_channel(this);
    delete this;
}

void ProtocolWrapperChannel::data_available(Dispatcher& r)
{
    if( closed ) {
        update_listen_status(r);
        return;
    }
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
    update_listen_status(r);
}

int ProtocolWrapperChannel::read_next_chunk()
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
    
void ProtocolWrapperChannel::write_possible(Dispatcher& r)
{
    if( closed ) {
        update_listen_status(r);
        return;
    }
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
        close();
    }
    update_listen_status(r);
}
    
void ProtocolWrapperChannel::update_listen_status(Dispatcher& r)
{
    if( closed ) {
        r.remove_channel(this);
        delete this;
        return;
    }
    if( read_eof || handler->is_finished() ) {
        r.listen_channel(this, Dispatcher::READ, false);
    } else {
        r.listen_channel(this, Dispatcher::READ, true);
    }
    
    if( write_eof || to_send.size() == 0 ) {
        r.listen_channel(this, Dispatcher::WRITE, false);
    } else {
        r.listen_channel(this, Dispatcher::WRITE, true);
    }
}

//
// ProtocolWrapperChannel::BufferedNonBlockingStream
//

    
ProtocolWrapperChannel::BufferedNonBlockingStream::BufferedNonBlockingStream(ProtocolWrapperChannel& b)
    : base(b) 
{}

intptr_t ProtocolWrapperChannel::BufferedNonBlockingStream::native() const 
{
    return base.channel->native();
}

void ProtocolWrapperChannel::BufferedNonBlockingStream::release()
{
    throw std::logic_error("ProtocolWrapperChannel::BufferedNonBlockingStream: release() not supported");
}

void ProtocolWrapperChannel::BufferedNonBlockingStream::close()
{
    base.close();
}

int ProtocolWrapperChannel::BufferedNonBlockingStream::seek(int pos, seek_origin origin)
{
    throw std::logic_error("ProtocolWrapperChannel::BufferedNonBlockingStream: seek() not supported");
}

int ProtocolWrapperChannel::BufferedNonBlockingStream::read(char* dest, int size)
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

int ProtocolWrapperChannel::BufferedNonBlockingStream::write(const char* data, int size)
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
void ProtocolWrapperChannel::BufferedNonBlockingStream::sync() 
{
    // not supported
}

