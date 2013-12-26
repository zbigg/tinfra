//
// Copyright (c) 2010-2011, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

// we implement this
#include "stream.h"

// we use this
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

void        stream_copy(input_stream& input, output_stream& out)
{
    static const int COPY_BUFFER_SIZE = 65536;
    char buffer[COPY_BUFFER_SIZE];
    int readed;
    
    while( (readed = input.read(buffer, sizeof(buffer))) > 0 ) {
        write_all(out, tstring(buffer, readed));
    }
}

} // end namespace tinfra

