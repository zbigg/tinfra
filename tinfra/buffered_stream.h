//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_buffered_stream_h_included
#define tinfra_buffered_stream_h_included

#include "stream.h"
#include <vector>
#include <memory>

namespace tinfra {

class buffered_input_stream: public tinfra::input_stream {
    tinfra::input_stream&     target;
    typedef std::vector<char> buffer_t;
    buffer_t buffer;
    
    buffer_t::iterator   buf_begin;
    buffer_t::iterator   buf_end;
    
    bool                 eof_readed;
public:
    buffered_input_stream(tinfra::input_stream& target, size_t size = 8192);
    ~buffered_input_stream();
    
    // implement tinfra::input_stream
    int  read(char* dest, int size);
    void close();
private:
    char*   ptr(buffer_t::iterator i ) { return & (*i); }
    char*   ptr(size_t idx)            { return & ( this->buffer[idx] ); }
    
    char*   ptr()   const { return & (*buf_begin); }
    bool    buf_empty() const { return buf_begin == buf_end; }
    size_t  buf_size()  const { return buf_end - buf_begin; }
    
    size_t consume_buffer(char* dest, size_t size);
    
    bool  fill_buffer();
};

class owning_buffered_input_stream: public input_stream {
    std::auto_ptr<tinfra::input_stream> delegate_;
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

} // end namespace tinfra

#endif // include guard
