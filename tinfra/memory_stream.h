//
// Copyright (c) 2013, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_memory_stream_h_included
#define tinfra_memory_stream_h_included

#include "tinfra/stream.h"
#include "tinfra/tstring.h"
#include <string>

namespace tinfra {

class memory_input_stream: public input_stream {
    const void* buffer_;
    const void* current_;
    
    size_t      remaining_size_;
    bool own_;
public:

    memory_input_stream(const void* buffer, size_t size, bool own);

    explicit memory_input_stream(tstring str);
    explicit memory_input_stream(std::string const& str);

    ~memory_input_stream();

    //
    // implement tinfra::input_stream
    //
    void close();
    int read(char* dest, int size);
};

//
// memory_output_stream
//
class memory_output_stream: public output_stream {
    std::string& out;
public:
    memory_output_stream(std::string& target);
    ~memory_output_stream();

    //
    // implement tinfra::output_stream
    //    
    virtual void close();
    virtual int write(const char* data, int size);
    using output_stream::write;

    virtual void sync();
};


//
// implementation
//

inline memory_input_stream::memory_input_stream(tstring str):
   buffer_(str.data()),
   current_(str.data()),
   remaining_size_(str.size()),
   own_(false)
{
}
inline memory_input_stream::memory_input_stream(std::string const& str):
   buffer_(str.data()),
   current_(str.data()),
   remaining_size_(str.size()),
   own_(false)
{
}

} // end namespace tinfra

#endif
