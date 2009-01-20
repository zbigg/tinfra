#ifndef tinfra_http_h__
#define tinfra_http_h__

#include <tinfra/tstring.h>
#include <tinfra/parser.h>
#include <tinfra/lazy_byte_consumer.h>

namespace tinfra { namespace http {

struct raw_parser_sink {
    virtual void request_line(
        tstring const& method, 
        tstring const& request_uri,
        tstring const& proto_version) = 0;
    virtual void status_line(
        tstring const& protocol_version,
        tstring const& status,
        tstring const& status_string) = 0;
    virtual void header(
        tstring const& name, 
        tstring const& value) = 0;
    virtual void finished_headers();
    virtual void content(
        tstring const& data,
        bool last) = 0;
    
    virtual ~raw_parser_sink() {}
};

class protocol_parser: public tinfra::parser {
public:
    enum parse_mode {
        SERVER,
        CLIENT
    };
    
    protocol_parser(raw_parser_sink&, parse_mode);

private:
    bool is_server() const { return mode_ == SERVER; }
    bool is_client() const { return mode_ == CLIENT; }
    
    // state updaters
    void setup_content_retrieval();
    void setup_initial_state();
    
    // byte handlers
    int request_line(tstring const& s);
    int header_line(tstring const& s);
    int content_bytes(tstring const& s);
    
    void handle_protocol_header(tstring const& name, tstring const& value);
    void check_content_length(size_t length);
    
    
    raw_parser_sink& sink_;
    parse_mode mode_;
    
    size_t parsed_content_length;
    size_t readed_content_length;
    
    tinfra::lazy_byte_consumer<protocol_parser> dispatch_helper_;
};

} } // end namespace tinfra::http

#endif // tinfra_http_h__
