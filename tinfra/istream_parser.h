//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_istream_sequence_parser_h__
#define __tinfra_istream_sequence_parser_h__

#include "tinfra/generator.h"
//#include "tinfra/trace.h"
#include "tinfra/sequence_parser.h"

namespace tinfra {

class istream_parser_impl_common {
protected:
    std::istream&  input_;
    std::string    buffer_;
    bool eof_readed_;
    bool eof_signaled_;
    
public:    
    istream_parser_impl_common(std::istream& in):
        input_(in), 
        eof_readed_(false),
        eof_signaled_(false)
    {}
protected:
    void retrieve_all_to_buffer() {
        while( true ) {
            char buf[1024];
            size_t readed = input_.read(buf, sizeof(buf));
            if( readed > 0 ) {
                put_to_buffer(buf, readed);
                return;
            }
            if( !input_ ) {
                eof_readed_ = true;
                return;
            }
            if( readed == 0 )
                return;
        }
    }
    
    void put_to_buffer(const char* b, size_t len)
    {
        buffer_.append(b, len);
    }
    
    void consume_buffer(size_t how_many)
    {
        buffer_.erase(0, how_many);
    }
    
    bool eof() {
        return eof_readed_ && eof_signaled_;
    }
};

class istream_parser: private istream_parser_impl_common 
{
    parser&     parser_;
    
public:
    istream_adapter(std::istream& in, parser& p): 
        istream_parser_impl_common(in), 
        parser_(p)
    {
    }
   
    void process() {
        while( !eof() ) {
            parse_buffered_content();
            
            if( maybe_reached_eof() ) 
                return;
        } 
    }
private:
    bool maybe_reached_eof()
    {
        if( eof_readed_ ) {
            parser_.eof(buffer_);
            eof_signaled_ = true;
            buffer_.clear();
            return true;
        }
        return false;
    }
    
    void parse_buffered_content()
    {
        this->retrieve_all_to_buffer();
        size_t processed;
        while( buffer_.size() != 0 ) {
            processed = parser_.process_input(buffer_);
            if( processed == 0 )
                return;
            consume_buffer(processed);            
        }
    }
};

} // end namespace tinfra

#endif // __tinfra_istream_sequence_parser_h___

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++
