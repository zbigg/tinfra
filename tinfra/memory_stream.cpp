#include "memory_stream.h"

namespace tinfra {
//
// memory_input_stream
//

memory_input_stream::memory_input_stream(const void* buffer, size_t size, bool own):
    buffer_(buffer),
    current_(buffer),
    remaining_size_(size),
    own_(own)
{
}

memory_input_stream::~memory_input_stream() {
    if( own_ ) {
        delete [] (char*)buffer_;
    }
}
void memory_input_stream::close()
{
}

int memory_input_stream::read(char* dest, int size)
{
    size_t read_size = size;
    if( read_size > remaining_size_ )
        read_size = remaining_size_;
    
    memcpy(dest, current_, read_size);
    
    remaining_size_ -= read_size;
    current_ = (char*)current_ + read_size;
    return read_size;
}

//
// memory_output_stream
//

memory_output_stream::memory_output_stream(std::string& o): 
    out(o)
{
}

memory_output_stream::~memory_output_stream()
{
}

void memory_output_stream::close() 
{
}

int memory_output_stream::write(const char* data, int size) { 
    out.append(data, size);
    return size;
}

void memory_output_stream::sync() { 
}

} // end namespace tinfra
