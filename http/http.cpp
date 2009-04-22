#include "http.h"

#include <tinfra/regexp.h>

namespace tinfra { namespace http {

namespace S {
    symbol content_length("content_length");
    symbol keep_alive("keep_alive");
}

using tinfra::tstring;

namespace {
    const tinfra::regexp http_request_line_re("^([A-Z]+)[ ]+([^ ]+)([ ]+([A-Z/0-9\\.]+))");
}

protocol_listener::~protocol_listener()
{
}

protocol_parser::protocol_parser(protocol_listener& pl, parse_mode pm)
    : sink_(pl), 
      mode_(pm),
      parsed_content_length(0), 
      readed_content_length(0),
      dispatch_helper_(*this)
{
    setup_initial_state();
}

int protocol_parser::process_input(tinfra::tstring const& input)
{
	int processed = dispatch_helper_.process(input);
	return processed;
}

void protocol_parser::eof(tinfra::tstring const& unparsed_input)
{
	int processed = dispatch_helper_.process(unparsed_input);
	// ok, what now ?
        processed;
}

void protocol_parser::setup_initial_state()
{
    dispatch_helper_.wait_for_delimiter("\r\n", &protocol_parser::request_line);
    parsed_content_length = tstring::npos;
}

int protocol_parser::request_line(tstring const& s) {
    dispatch_helper_.wait_for_delimiter("\r\n", &protocol_parser::header_line);
            
    // TODO: store/consume request-line
    tinfra::static_tstring_match_result<5> parse_result;
    
    if( http_request_line_re.matches(s, parse_result)) {
        sink_.request_line(
            parse_result.groups[1], 
            parse_result.groups[2], 
            parse_result.groups[4]);
    } else {
        throw std::runtime_error("HTTP: bad request-line");
    }
    return s.size();
}

int protocol_parser::header_line(tstring const& s) {
    if( s.size() > 2 ) {
        size_t pos = s.find_first_of(':');
        if( pos == tstring::npos )
            throw std::runtime_error("HTTP: bad header");
        
        tstring name(s.data(), pos-1);
        tstring value(s.data() + pos, s.size()-pos-1);
        
        sink_.header(name, value);
        
        try {
            handle_protocol_header(name, value);
        } catch( std::exception const& e) {
            throw std::runtime_error("HTTP: bad header");
        }
    } else if( s.size() == 2 ) {
        sink_.finished_headers();
        // headers ENDED
        // somehow find content-length
        readed_content_length = 0;
        
        setup_content_retrieval();
    }
    return s.size();
}

void protocol_parser::setup_content_retrieval()
{        
    size_t remaining_size = parsed_content_length - readed_content_length;
    
    if( parsed_content_length == tstring::npos ) {
        dispatch_helper_.next( &protocol_parser::content_bytes );
    } if( remaining_size > 0 ) {
        dispatch_helper_.next( &protocol_parser::content_bytes );
    } else if( false /* && is_keep_alive() */ ) {
        setup_initial_state();
    } else {
        dispatch_helper_.finish();
    }
}
const tstring CONTENT_LENGTH = "content-length";

void protocol_parser::handle_protocol_header(tstring const& name, tstring const& value)
{
    if( name == CONTENT_LENGTH ) { // TODO: case
        int n;
        tinfra::from_string(value, n);
        if( n < 0 ) {
            throw std::runtime_error("invalid (negative) content-length");
        }
        parsed_content_length = n;                        
        check_content_length(parsed_content_length);
    }
}

void protocol_parser::check_content_length(size_t length)
{
    const size_t max_content_length = 1024*1024*10;
    if( length >= max_content_length ) {
        throw std::runtime_error("to big content-length");
    }
}

int protocol_parser::content_bytes(tstring const& s) {
    size_t remaining_size = parsed_content_length - readed_content_length;        
    size_t current_size = std::min(remaining_size, s.size());
    
    readed_content_length += current_size;
    
    check_content_length(readed_content_length);
    
    bool last = (readed_content_length == parsed_content_length);        
    sink_.content(tstring(s.data(), current_size), last);
            
    setup_content_retrieval();
    return current_size;
}
    
} } // end namespace tinfra::http

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

