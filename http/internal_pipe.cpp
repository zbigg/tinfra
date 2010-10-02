#include "internal_pipe.h" // we implement this

#include <tinfra/thread.h>
#include <deque>

namespace tinfra {

struct internal_pipe::implementation_detail
{
    tinfra::monitor monitor;
    int  requested_buffer_size; // read_only
    bool closed;
    std::deque<char> buffer;
public:
    implementation_detail(int _buffer_size):
        requested_buffer_size(_buffer_size),
        closed(false)
    {
    }
    
    int read(char* dest, int requested_read_size)
    {
        tinfra::synchronizator lock(this->monitor);
        
        if( buffer.size() == 0 && closed )
            return 0; // EOF
        
        while (buffer.size() == 0 ) {
            lock.wait();
        }
        
        const int actual_read_size = std::min(requested_read_size);
        
        buffer.remove(buffer.begin()
        return actual;
        
    }
    
    int write(const char* data, int size)
    {
        tinfra::synchronizator lock(this->monitor);
        if( closed ) 
            throw std::logic_error("internal_pipe: attempt to write() after close()");
        
        const bool should_signal = (buffer.size() == 0);
        
        
        if( should_signal )
            lock.broadcast();
    }
    
    void close()
    {
        this->closed = true;
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

