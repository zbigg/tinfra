#include "http.h"

#include <tinfra/regexp.h>

#include <limits>

namespace tinfra { namespace http {

namespace S {
    symbol content_length("content_length");
    symbol keep_alive("keep_alive");
}

using tinfra::tstring;

namespace {
    const tinfra::regexp http_request_line_re("^([A-Z]+)[ ]+([^ ]+)([ ]+([A-Z/0-9\\.]+))");
}

struct protocol_error: public std::runtime_error {
public:
    protocol_error(const char* s):
        std::runtime_error(s)
    {}
};

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
    TINFRA_TRACE_VAR(unparsed_input);
    switch( state ) {
    case EXPECTING_RESET:
        return;
    case EXPECTING_CONTENT:
        if( unparsed_input.size() > 0 ) {
            // TODO:
            // if we
            sink_.content(unparsed_input, true);
        }
        state = EXPECTING_RESET;
        return;
    case EXPECTING_REQUEST_LINE:
        throw protocol_error("expecting request line");
    case EXPECTING_RESPONSE_LINE:
        throw protocol_error("expecting response line");
    case EXPECTING_HEADER:
        throw protocol_error("expecting header");
    
    }
}

void protocol_parser::setup_initial_state()
{
    dispatch_helper_.wait_for_delimiter("\r\n", &protocol_parser::request_line);
    state = (mode_ == SERVER) ? EXPECTING_REQUEST_LINE
                              : EXPECTING_RESPONSE_LINE;
    parsed_content_length = tstring::npos;
}

int protocol_parser::request_line(tstring const& s) {
    
    const size_t end = s.find("\r\n");
    if( end == tstring::npos )
        throw protocol_error("bad request line");
    const tstring request_line = s.substr(0, end);
    
    // TODO: store/consume request-line
    tinfra::static_tstring_match_result<5> parse_result;
    
    if( http_request_line_re.matches(request_line, parse_result)) {
        sink_.request_line(
            parse_result.groups[1], 
            parse_result.groups[2], 
            parse_result.groups[4]);
    } else {
        throw protocol_error("HTTP: bad request-line");
    }
    
    dispatch_helper_.wait_for_delimiter("\r\n", &protocol_parser::header_line);
    state = EXPECTING_HEADER;
    return request_line.size()+2;
}

int protocol_parser::header_line(tstring const& s) {
    const size_t end = s.find("\r\n");
    if( end == tstring::npos )
        throw protocol_error("bad header");
    if( end == 0 ) {
         sink_.finished_headers();
        // headers ENDED
        // somehow find content-length
        readed_content_length = 0;
        
        setup_content_retrieval();
        return 2;
    }
    
    const tstring header_line = s.substr(0, end);
    const size_t pos = header_line.find_first_of(':');
    
    if( pos == tstring::npos )
        throw protocol_error("bad header");
    
    tstring name  = header_line.substr(0, pos);
    tstring value = header_line.substr(pos+2);
    
    sink_.header(name, value);
    
    try {
        handle_protocol_header(name, value);
    } catch( std::exception const& e) {
        throw protocol_error("bad header");
    }
    dispatch_helper_.wait_for_delimiter("\r\n", &protocol_parser::header_line);
    state = EXPECTING_HEADER;
    
    return header_line.size()+2;
}

void protocol_parser::setup_content_retrieval()
{        
    size_t remaining_size = parsed_content_length - readed_content_length;
    
    if( false /* && is_keep_alive() */ ) {
        state = EXPECTING_RESET;
        setup_initial_state();
        return;
    }
    
    state = EXPECTING_CONTENT;
    dispatch_helper_.wait_for_anything( &protocol_parser::content_bytes );
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
        return;
    }
    
}

void protocol_parser::check_content_length(size_t length)
{
    const size_t max_content_length = 1024*1024*10;
    if( length >= max_content_length ) {
        throw protocol_error("to big content-length");
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

static void write_headers(std::ostream& out, std::vector<request_header_entry> const& hv, size_t content_length)
{
    using std::numeric_limits;
    const bool use_content_length = (content_length != numeric_limits<size_t>::max() );
    
    std::vector<request_header_entry>::const_iterator i = hv.begin();
    while( i != hv.end() ) {
        out << i->name << ": " << i->value << "\r\n";
    }
    
    if( use_content_length ) {
        out << "Content-Length: " << content_length << "\r\n";
    }
}

//void write(tinfra::io::stream*, response_header_data const&, optional<size_t> const& content_length)
void write(tinfra::io::stream* out, response_header_data const& rhd, size_t content_length)
{
    const char* proto_string = rhd.proto == HTTP_1_0 ? "HTTP/1.0"
                                                     : "HTTP/1.1";
    std::ostringstream formatter;
    formatter << proto_string << " " << rhd.status << " " << rhd.status_message << "\r\n";
    
    write_headers(formatter, rhd.headers, content_length);
    
    std::string const& r = formatter.str();
    out->write(r.data(), r.size());
}

//void write(tinfra::io::stream*, request_header_data const&, optional<size_t> const& content_length)
void write(tinfra::io::stream* out, request_header_data const& rhd, size_t content_length)
{
    const char* proto_string = rhd.proto == HTTP_1_0 ? "HTTP/1.0"
                                                     : "HTTP/1.1";
    
    std::ostringstream formatter;
    formatter << rhd.method << " " << rhd.request_uri << " " << proto_string << "\r\n";
    
    write_headers(formatter, rhd.headers, content_length);
    
    std::string const& r = formatter.str();
    out->write(r.data(), r.size());
}

void write(tinfra::io::stream* out, request_data const& rf)
{
    write(out, rf.header, rf.content.size());
    
    out->write(rf.content.data(), rf.content.size());
}

void write(tinfra::io::stream* out, response_data const& rd)
{
    write(out, rd.header, rd.content.size());
    
    out->write(rd.content.data(), rd.content.size());
}

} } // end namespace tinfra::http

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

