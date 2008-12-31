//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_parser_h__
#define __tinfra_parser_h__

#include "tinfra/generator.h"

namespace tinfra {

template <typename T>
class parser {
public:
	virtual int   process_input(tinfra::tstring const& input) = 0;
	virtual void  eof(tinfra::tstring const& unparsed_input) = 0;
        virtual bool  get_result(T& r) = 0; 
};

template <typename T>
class istream_parser_adaptor: public generator_impl<istream_parser_adaptor<T>, T> {
    std::streambuf&  input_;
    parser<T>&       parser_;
    std::string      buffer_;
public:
    istream_parser_adaptor(std::streambuf& in, parser<T>& p): 
        input_(in), 
        parser_(p),
        eof_readed_(false),
        eof_signaled_(false)
    {
    }
   
    bool fetch_next(T& r) {
        while( !eof() ) {
            if( parse_buffer(r) )
                return true;
            
            fill_up_buffer();
            
            if( eof_readed_ ) {
                parser_.eof(buffer_);
                if( parser_.get_result(r) )
                    return true;
                eof_signaled_ = true;
            }
        } 
        return false;
    }
private:
    bool parse_buffer(T& result)
    {
        while( input_.in_avail() ) {
            char buf[1024];
            size_t readed = input_.sgetn(buf, input_.in_avail());
            put_to_buffer(buf, readed);
        }
        size_t processed;
        while( buffer_.size() != 0 ) {
            processed = parser_.process_input(buffer_);
            if( processed != 0 ) {
                consume_buffer(processed);
                if( parser_.get_result(result) )
                    return true;
            } else {
                // nothing processed so need more bytes
                break;
            }
        }
        return false;
    }
    void fill_up_buffer()
    {
        int c = input_.snextc();
        if( c == EOF ) {
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
    bool eof_readed_;
    bool eof_signaled_;
};

} // end namespace tinfra

#endif //__tinfra_parser_h__

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++
