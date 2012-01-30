#include "buffered_stream.h"

namespace tinfra {

buffered_input_stream::buffered_input_stream(tinfra::input_stream& target, size_t size):
        target(target),
        buffer(size),
        buf_begin(buffer.begin()),
        buf_end(buffer.begin()),
        eof_readed(false)
{
}

buffered_input_stream::~buffered_input_stream()
{
}

int buffered_input_stream::read(char* dest, int size)
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

void buffered_input_stream::close()
{
    this->eof_readed = true;
    this->buf_begin = this->buf_end = this->buffer.begin();
}
/* inline
char*   ptr(buffer_t::iterator i ) { return & (*i); }
char*   ptr(size_t idx)            { return & ( this->buffer[idx] ); }

char*   ptr()   const { return & (*buf_begin); }
bool    buf_empty() const { return buf_begin == buf_end; }
size_t  buf_size()  const { return buf_end - buf_begin; }
*/
size_t buffered_input_stream::consume_buffer(char* dest, size_t size)
{
    const size_t consumed_size = std::min(buf_size(), size);
    if( consumed_size > 0 ) {
        std::memcpy(dest, ptr(buf_begin), consumed_size);
        buf_begin += consumed_size;
    }
    return consumed_size;
}

bool  buffered_input_stream::fill_buffer() 
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

} // end namespace tinfra

