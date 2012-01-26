//
// Copyright (c) 2010-2011, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

// we implement this
#include "stream.h"

// we use this
#include "tinfra/io/stream.h" // for tinfra::io::stream, open_file
#include <memory>             // for auto_ptr
#include <algorithm>          // for std::min
#include <stdlib.h>           // for ::memmove
namespace tinfra {

using std::auto_ptr;
    
input_stream::~input_stream() 
{
}

output_stream::~output_stream()
{
}

int output_stream::write(tstring const& data)
{
    return this->write(data.data(), data.size());
}

class buffered_input_stream {
    tinfra::io::stream& target;
    typedef std::vector<char> buffer_t;
    buffer_t buffer;
    
    buffer_t::iterator   buf_begin;
    buffer_t::iterator   buf_end;
    
    bool                 eof_readed;
public:
    buffered_input_stream(tinfra::io::stream& target, size_t size = 8192) :
        target(target),
        buffer(size),
        buf_begin(buffer.begin()),
        buf_end(buffer.begin()),
        eof_readed(false)
    {
    }
    int read(char* dest, int size)
    {
        size_t readed_size = 0;
        const size_t initial_consumed = consume_buffer(dest, size);
        if( initial_consumed == size )
            return size;
        
        readed_size += initial_consumed;
        size -= initial_consumed;
        dest += initial_consumed;
        
        if( eof_readed ) { // someone readed EOF before, so
                           // no reason to try to read anything more
            return readed_size;
        }
        
        if( size > buffer.size() ) {
            // if requested size is bigger than buffer
            // then read directly into target buffer
            int r = target.read(dest, size);
            if( r == 0 ) {
                eof_readed = true;
            }
            readed_size += r;
            return readed_size;
        } else {
            // first, fill buffer so we minimize 
            // reads
            if( !fill_buffer() ) {
                // after filling buffer is still empty
                // so it can mean one, EOF
                return readed_size;
            }
            
            // finally consume rest of remaining_read
            const size_t second_consumed = consume_buffer(dest, size);
            return initial_consumed + second_consumed;
        } 
        return readed_size;
    }
private:
    char*   ptr(buffer_t::iterator i ) { return & (*i); }
    char*   ptr(size_t idx)            { return & ( this->buffer[idx] ); }
    
    char*   ptr()   const { return & (*buf_begin); }
    bool    buf_empty() const { return buf_begin == buf_end; }
    size_t  buf_size()  const { return buf_end - buf_begin; }
    
    size_t consume_buffer(char* dest, size_t size)
    {
        const size_t consumed_size = std::min(buf_size(), size);
        if( consumed_size > 0 ) {
            std::memcpy(dest, ptr(buf_begin), consumed_size);
            buf_begin += consumed_size;
        }
        return consumed_size;
    }
    
    bool  fill_buffer() 
    {
        const size_t initial_size = this->buf_size();
        if( eof_readed ) 
            return ! this->buf_empty(); 
        
        if( this->buf_begin != this->buffer.begin() ) {
            
            std::memmove(ptr(0), ptr(this->buf_begin), initial_size);
            this->buf_begin = this->buffer.begin();
            this->buf_end   = this->buffer.begin() + initial_size;
        }
        const size_t remaining_read = this->buffer.size() - initial_size;
        
        const int r = target.read(ptr(this->buf_end), remaining_read);
        if( r == 0 ) {
            eof_readed = true;
        }
        this->buf_end += r;
        
        return ! this->buf_empty();
    }
};

class old_input_stream_adapter: public input_stream {
    auto_ptr<tinfra::io::stream> delegate_;
    buffered_input_stream buffer;
public:
    old_input_stream_adapter(tinfra::io::stream* base, size_t buffer_size):
        delegate_(base),
        buffer(*base, buffer_size)
    {
    }
    
    //
    // implement tinfra::input_stream
    //
    void close()
    {
        delegate_->close();
    }

    int read(char* dest, int size)
    {
        return buffer.read(dest, size);
    }
};

const size_t DEFAULT_BUFFER_SIZE = 8192;

auto_ptr<input_stream> create_file_input_stream(tstring const& name, size_t buffer_size)
{
    const std::string tmp_name(name); // hack to satisfy old tinfra::io api
    
    tinfra::io::stream* delegate = tinfra::io::open_file(tmp_name.c_str(), std::ios::in);
    
    return auto_ptr<input_stream>(new old_input_stream_adapter(delegate, buffer_size));
}

auto_ptr<input_stream> create_file_input_stream(tstring const& name)
{
    const std::string tmp_name(name); // hack to satisfy old tinfra::io api
    
    tinfra::io::stream* delegate = tinfra::io::open_file(tmp_name.c_str(), std::ios::in);
    
    return auto_ptr<input_stream>(new old_input_stream_adapter(delegate, DEFAULT_BUFFER_SIZE));
}

//
// memory_input_stream
//
class memory_input_stream_impl: public input_stream {
    const void* buffer_;
    const void* current_;
    
    size_t      remaining_size_;
    bool own_;
public:
    memory_input_stream_impl(const void* buffer, size_t size, bool own):
        buffer_(buffer),
        current_(buffer),
        remaining_size_(size),
        own_(own)
    {
    }
    
    ~memory_input_stream_impl() 
    {
        if( own_ ) {
            delete [] (char*)buffer_;
        }
    }
    //
    // implement tinfra::input_stream
    //
    void close()
    {
    }

    int read(char* dest, int size)
    {
        size_t read_size = size;
        if( read_size > remaining_size_ )
            read_size = remaining_size_;
        
        memcpy(dest, current_, read_size);
        
        remaining_size_ -= read_size;
        current_ = (char*)current_ + read_size;
        return read_size;
    }
};

auto_ptr<input_stream>  create_memory_input_stream(const void* buffer, size_t size, memory_strategy buffer_strategy)
{
    const void* buffer2 = buffer;
    if( buffer_strategy == COPY_BUFFER ) {
        char* tmp = new char[size];
        memcpy(tmp, buffer, size);
        buffer2 = tmp;
    }
    const bool STREAM_OWNS_BUFFER = (buffer_strategy == COPY_BUFFER);
    
    return auto_ptr<input_stream>( new memory_input_stream_impl(buffer2, size, STREAM_OWNS_BUFFER));
}

//
// memory_output_stream
//
class memory_output_stream_impl: public output_stream {
    std::string& out;
public:
    memory_output_stream_impl(std::string& o): out(o) {}
    
    //
    // implement tinfra::output_stream
    //    
    virtual void close() { }
    
    virtual int write(const char* data, int size) { 
        out.append(data, size);
        return size;
    }
    
    virtual void sync() { }
};

std::auto_ptr<output_stream> create_memory_output_stream(std::string& out)
{
    return auto_ptr<output_stream> (new memory_output_stream_impl(out));
}

class old_output_stream_adapter: public output_stream {
    auto_ptr<tinfra::io::stream> delegate_;
public:
    old_output_stream_adapter(tinfra::io::stream* base):
        delegate_(base)
    {
    }
    
    //
    // implement tinfra::output_stream
    //
    
    virtual void close()
    {
        delegate_->close();
    }
    
    virtual int write(const char* data, int size)
    {
        return delegate_->write(data, size);
    }
    
    virtual void sync()
    {
        delegate_->sync();
    }
};

auto_ptr<output_stream> create_file_output_stream(tstring const& name, int mode)
{
    std::ios::openmode flags = std::ios::out;
    if( (mode & APPEND) == APPEND) 
        flags |= std::ios::app;
    if( (mode & TRUNCATE) == TRUNCATE) 
        flags |= std::ios::trunc;
    
    const std::string tmp_name(name); // hack to satisfy old tinfra::io api
    
    tinfra::io::stream* delegate = tinfra::io::open_file(tmp_name.c_str(), flags);
    
    return auto_ptr<output_stream>(new old_output_stream_adapter(delegate));
}

std::string read_all(input_stream* input)
{
    static const int COPY_BUFFER_SIZE = 65536;

    char buffer[COPY_BUFFER_SIZE];
    std::vector<char> tmp;
    int readed;
    
    while( (readed = input->read(buffer, sizeof(buffer))) > 0 ) {
        tmp.insert(tmp.end(),
            buffer, buffer + readed);
    }
    
    return std::string(tmp.begin(), tmp.end());
}

} // end namespace tinfra
