// we implement this
#include "stream.h"

// we use this
#include "tinfra/io/stream.h" // for tinfra::io::stream, open_file
#include <memory>             // for auto_ptr

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


auto_ptr<input_stream> create_file_input_stream(tstring const& name)
{
    const std::string tmp_name(name); // hack to satisfy old tinfra::io api
    
    tinfra::io::stream* delegate = tinfra::io::open_file(tmp_name.c_str(), std::ios::in);
    
    return auto_ptr<input_stream>(new old_input_stream_adapter(delegate));
}

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
        int read_size = size;
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
