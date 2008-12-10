#include <stdexcept>
#include <tinfra/regexp.h>
#include <tinfra/tinfra_lex.h>

#include "lazy_protocol.h"

using tinfra::tstring;
using tinfra::regexp;

struct http_request_processor {
    virtual void request_line(
        tstring const& method, 
        tstring const& request_uri,
        tstring const& proto_version) = 0;
    virtual void header(
        tstring const& name, 
        tstring const& value) = 0;
    virtual void finished_header();
    virtual void content(
        tstring const& data,
        bool last) = 0;
    
    virtual ~http_request_processor() {}
};

class http_request_parser: public tinfra::lazy_protocol  {
    
    http_request_parser(http_request_processor& p)
        : processor(p)
    {
        setup_initial_state();
    }

    bool is_keep_alive() const { return false; }
    
private:
    http_request_processor& processor;

    static const tinfra::regexp http_11_request_line_re;
    static const tinfra::regexp http_10_request_line_re;

    int request_line(tstring const& s) {        
        wait_for_delimiter("\r\n", 
            make_step_method(&http_request_parser::header_line));
                
        // TODO: store/consume request-line
        tinfra::static_tstring_match_result<4> parsed;
        
        if( http_11_request_line_re.matches(s, parsed)) {
            processor.request_line(parsed.groups[1], parsed.groups[2], parsed.groups[3]);
        } else {
            tinfra::static_tstring_match_result<3> parsed_old;
            if( http_10_request_line_re.matches(s, parsed_old) ) {
                processor.request_line(parsed_old.groups[1], parsed_old.groups[2], "");
            } else {
                throw std::runtime_error("HTTP: bad request-line");
            }
        }
        return s.size();
    }
    
    int header_line(tstring const& s) {
        if( s.size() > 2 ) {
            size_t pos = s.find_first_of(':');
            if( pos == tstring::npos )
                throw std::runtime_error("HTTP: bad header");
            
            tstring name(s.data(), pos-1);
            tstring value(s.data() + pos, s.size()-pos-1);
            
            processor.header(name, value);
            
            try {
                handle_protocol_header(name, value);
            } catch( std::exception const& e) {
                throw std::runtime_error("HTTP: bad header");
            }
        } else if( s.size() == 2 ) {
            processor.finished_header();
            // headers ENDED
            // somehow find content-length
            readed_content_length = 0;
            
            setup_content_retrieval();
        }
        return s.size();
    }
    
    void setup_initial_state()
    {
        wait_for_delimiter("\r\n",
            make_step_method(&http_request_parser::request_line));
        requested_content_length = tstring::npos;
    }
    
    void setup_content_retrieval()
    {        
        size_t remaining_size = requested_content_length - readed_content_length;
        
        if( requested_content_length == tstring::npos ) {
            next(make_step_method(&http_request_parser::content_bytes));
        } if( remaining_size > 0 ) {
            next(make_step_method(&http_request_parser::content_bytes));
        } else if( is_keep_alive() ) {
            setup_initial_state();
        } else {
            finish();
        }
    }
    
    void handle_protocol_header(tstring const& name, tstring const& value)
    {        
        
        if( name == "content-length" ) {
            int n;
            tinfra::from_string(value, n);
            if( n < 0 ) {
                throw std::runtime_error("invalid (negative) content-length");
            }
            requested_content_length = n;                        
            check_content_length(requested_content_length);
        }
    }
    
    void check_content_length(size_t length)
    {
        const size_t max_content_length = 1024*1024*10;
        if( length >= max_content_length ) {
            throw std::runtime_error("to big content-length");
        }
    }
    
    int content_bytes(tstring const& s) {
        size_t remaining_size = requested_content_length - readed_content_length;        
        size_t current_size = std::min(remaining_size, s.size());
        
        readed_content_length += current_size;
        
        check_content_length(readed_content_length);
        
        bool last = (readed_content_length == requested_content_length);        
        processor.content(tstring(s.data(), current_size), last);
                
        setup_content_retrieval();
        return current_size;
    }

    size_t requested_content_length;
    size_t readed_content_length;
};

//
// http_request_parser
//

const tinfra::regexp http_request_parser::http_11_request_line_re("^([A-Z]+)[ ]+([^ ]+)[ ]+([A-Z/0-9\\.]+)");
const tinfra::regexp http_request_parser::http_10_request_line_re("^([A-Z]+)[ ]+([^ ]+)");

int main()
{
    return 0;
}


