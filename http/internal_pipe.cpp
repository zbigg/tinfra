#include "internal_pipe.h" // we implement this

#include <tinfra/thread.h>
#include <deque>        // implementation underlying buffer
#include <iterator>     // for std::advance
#include <algorithm>    // for std::copy

namespace tinfra {

struct internal_pipe::implementation_detail
{
    tinfra::monitor  monitor;
    //int              requested_buffer_size; // read_only, not implemented!!!
    bool             closed;
    std::deque<char> buffer;
public:
    implementation_detail(int _buffer_size):
        //requested_buffer_size(_buffer_size),
        closed(false)
    {
    }
    
    int read(char* dest, int requested_read_size)
    {
        tinfra::synchronizator lock(this->monitor);
        
        while (this->buffer.size() == 0 ) {
            if( this->closed )
                return 0; // EOF
            this->monitor.wait();
        }
        
        const int actual_read_size = std::min(buffer.size, requested_read_size);
        
        deque::iterator copy_begin = this->buffer.begin();
        deque::iterator copy_end = this->buffer.begin();
        advance(copy_end, actual_read_size);
        std::copy(copy_begin, copy_end, dest);
        this->buffer.erase(copy_begin, copy_end);
        return actual_read_size;        
    }
    
    int write(const char* data, int size)
    {
        tinfra::synchronizator lock(this->monitor);
        
        if( this->closed ) // TODO: it should be TINFRA_LOGIC_INVARIANT
            throw std::logic_error("internal_pipe: attempt to write() after close()");
        
        const bool should_signal = (this->buffer.size() == 0);
        
        this->buffer.insert(this->buffer.end(), data, data+size);
        
        if( should_signal )
            this->monitor.broadcast();
        
        return size;
    }
    
    void close()
    {
        tinfra::synchronizator lock(this->monitor);
        this->closed = true;
        if( this->buffer.size() == 0 )
            this->monitor.broadcast();
    }
};

internal_pipe::internal_pipe(int buffer_size)
    : impl(new implementation_detail(this, buffer_size))
{
}

int internal_pipe::read(char* dest, int size)
{
    return impl->read(dest, size);
}

int internal_pipe::write(const char* data, int size)
{
    return impl->write(dest, size);
}

void internal_pipe::sync()
{
    // nothing to sync here
}

void internal_pipe::close()
{
    impl->close();
}

} // end namespace tinfra

