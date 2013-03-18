//
// Copyright (c) 2010-2011, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

// we implement this
#include "stream.h"

// we use this
#include "tinfra/io/stream.h" // for tinfra::io::stream, open_file
#include "tinfra/fail.h"      // for tinfra::fail
#include "tinfra/fmt.h"       // for tinfra::tsprintf
#include "buffered_stream.h"
#include "memory_stream.h"

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

class old_input_stream_adapter: public input_stream {
    auto_ptr<tinfra::io::stream> delegate_;
public:
    old_input_stream_adapter(tinfra::io::stream* base):
        delegate_(base)
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
        return delegate_->read(dest, size);
    }
};

class owning_buffered_input_stream: public input_stream {
    auto_ptr<tinfra::input_stream> delegate_;
    buffered_input_stream          buffer;
public:
    owning_buffered_input_stream(input_stream* base, size_t buffer_size):
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
    tinfra::input_stream* delegate2 = new old_input_stream_adapter(delegate);
    
    return auto_ptr<input_stream>(new owning_buffered_input_stream(delegate2, buffer_size));
}

auto_ptr<input_stream> create_file_input_stream(tstring const& name)
{
    const std::string tmp_name(name); // hack to satisfy old tinfra::io api
    
    tinfra::io::stream* delegate = tinfra::io::open_file(tmp_name.c_str(), std::ios::in);
    tinfra::input_stream* delegate2 = new old_input_stream_adapter(delegate);
    
    return auto_ptr<input_stream>(new owning_buffered_input_stream(delegate2, DEFAULT_BUFFER_SIZE));
}

auto_ptr<input_stream>  create_memory_input_stream(const void* buffer, size_t size, memory_strategy buffer_strategy)
{
    const void* buffer2 = buffer;
    if( buffer_strategy == COPY_BUFFER ) {
        char* tmp = new char[size];
        memcpy(tmp, buffer, size);
        buffer2 = tmp;
    }
    const bool STREAM_OWNS_BUFFER = (buffer_strategy == COPY_BUFFER);
    
    return auto_ptr<input_stream>( new memory_input_stream(buffer2, size, STREAM_OWNS_BUFFER));
}

std::auto_ptr<output_stream> create_memory_output_stream(std::string& out)
{
    return auto_ptr<output_stream> (new memory_output_stream(out));
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

std::string read_all(input_stream& input)
{
    static const int COPY_BUFFER_SIZE = 65536;

    char buffer[COPY_BUFFER_SIZE];
    std::vector<char> tmp;
    int readed;
    
    while( (readed = input.read(buffer, sizeof(buffer))) > 0 ) {
        tmp.insert(tmp.end(),
            buffer, buffer + readed);
    }
    
    return std::string(tmp.begin(), tmp.end());
}

void        write_all(output_stream& output, tstring const& data)
{
    using tinfra::fail;
    using tinfra::tsprintf;
    
    size_t to_write              = data.size();
    size_t buffer_to_write_index = 0;
    
    while( to_write > 0 ) {
        const int w = output.write(data.data() + buffer_to_write_index, to_write);
        if( to_write != 0 && w == 0 ) {
            fail(tsprintf("unable to save data, %i bytes left", to_write), 
                 "write() unexpectedly returned 0"); 
        }
        to_write -= w;
        buffer_to_write_index += w;
    }
}

} // end namespace tinfra

