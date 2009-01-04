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

class istream_sequence_parser_impl_common {
protected:
    std::istream&  input_;
    std::string    buffer_;
    bool eof_readed_;
    bool eof_signaled_;
    
public:    
    istream_sequence_parser_impl_common(std::istream& in):
        input_(in), 
        eof_readed_(false),
        eof_signaled_(false)
    {}
protected:
    void retrieve_all_to_buffer() {
        while( true ) {
            char buf[1024];
            size_t readed = input_.readsome(buf, sizeof(buf));
            if( !input_ ) {
                eof_readed_ = true;
                return;
            }
            //TINFRA_TRACE_VAR(readed);
            if( readed == 0 )
                break;
            put_to_buffer(buf, readed);
        }
    }
    void fill_up_buffer()
    {
        //TINFRA_TRACE_MSG("csv: reading byte");
        char c;
        input_.read(&c, 1);
        if( !input_ ) {
            eof_readed_ = true;
        } else {
            buffer_.append(1, c);
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

template <typename T>
class istream_sequence_parser: public generator_impl<istream_sequence_parser<T>, T>, 
                              private istream_sequence_parser_impl_common 
{
    sequence_parser<T>&     parser_;
    
public:
    istream_sequence_parser(std::istream& in, sequence_parser<T>& p): 
        istream_sequence_parser_impl_common(in), 
        parser_(p)
    {
    }
   
    bool fetch_next(T& r) {
        while( !eof() ) {
            if( parse_buffered_content(r) )
                return true;
            
            fill_up_buffer();
            
            if( maybe_reached_eof(r) ) 
                return true;
        } 
        return false;
    }
private:
    bool maybe_reached_eof(T& r)
    {
        if( eof_readed_ ) {
            parser_.eof(buffer_);
            eof_signaled_ = true;
            buffer_.clear();
            if( parser_.get_result(r) )
                return true;
        }
        return false;
    }
    
    bool parse_buffered_content(T& result)
    {
        //TINFRA_TRACE_VAR(input_.in_avail());
        this->retrieve_all_to_buffer();
        size_t processed;
        while( buffer_.size() != 0 ) {
            processed = parser_.process_input(buffer_);
            if( processed != 0 ) {
                consume_buffer(processed);
                if( parser_.get_result(result) )
                    return true;
            } else {
                // nothing processed so need more bytes
                return false;
            }
        }
        return false;
    }
};

} // end namespace tinfra

#endif // __tinfra_istream_sequence_parser_h___

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++
