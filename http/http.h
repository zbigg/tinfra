#ifndef tinfra_http_h_included_
#define tinfra_http_h_included_

#include <tinfra/tstring.h>
#include <tinfra/parser.h>
#include <tinfra/lazy_byte_consumer.h>
#include <tinfra/symbol.h>
#include <tinfra/io/stream.h>

//#include <tinfra/optional.h>    

namespace tinfra { namespace http {

enum protocol_version {
    HTTP_1_0,
    HTTP_1_1
};

struct request_header_entry {
    std::string       name;
    std::string       value;
};

struct request_header_data {
    std::string       method;
    std::string       request_uri;
    protocol_version  proto;
    
    std::vector<request_header_entry> headers;
};

struct request_data {
    request_header_data header;
    std::string         content;
};

struct response_header_data {
    protocol_version  proto;
    int               status;
    std::string       status_message;
    
    std::vector<request_header_entry> headers;
};

struct response_data {
    response_header_data header;
    std::string   content;
};

//void write(tinfra::io::stream*, response_header_data const&, optional<size_t> const& content_length);
//void write(tinfra::io::stream*, request_header_data const&, optional<size_t> const& content_length);

void write(tinfra::io::stream*, response_header_data const&, size_t content_length);
void write(tinfra::io::stream*, request_header_data const&, size_t content_length);

void write(tinfra::io::stream*, request_data const&);
void write(tinfra::io::stream*, response_data const& d);

namespace S {
    extern symbol content_length;
    extern symbol keep_alive;
}

struct protocol_listener {
    virtual void request_line(
        tstring const& method, 
        tstring const& request_uri,
        tstring const& protocol_version) = 0;
    virtual void response_line(
        tstring const& protocol_version,
        tstring const& status,
        tstring const& status_string) = 0;
    virtual void header(
        tstring const& name, 
        tstring const& value) = 0;
    virtual void finished_headers() {}
    virtual void content(
        tstring const& data,
        bool last) = 0;
    
    virtual ~protocol_listener(); 
};

class protocol_parser: public tinfra::parser {
public:
    enum parse_mode {
        SERVER,
        CLIENT
    };
    
    protocol_parser(protocol_listener&, parse_mode);

    int   process_input(tinfra::tstring const& input);
    void  eof(tinfra::tstring const& unparsed_input);

private:
    bool is_server() const { return mode_ == SERVER; }
    bool is_client() const { return mode_ == CLIENT; }
    
    enum {
        EXPECTING_RESPONSE_LINE,
        EXPECTING_REQUEST_LINE,
        EXPECTING_HEADER,
        EXPECTING_TRAILER,
        EXPECTING_CONTENT,
        EXPECTING_RESET
    } state;
    
    enum {
        IDENTITY,
        CHUNKED
    } transfer_encoding;
    
    // state updaters
    void setup_content_retrieval();
    void setup_initial_state();

    // byte handlers
    int request_line(tstring const& s);
    int header_line(tstring const& s);
    int trailer_line(tstring const& s);
    int content_bytes(tstring const& s);
    
    void handle_protocol_header(tstring const& name, tstring const& value);
    void check_message_length(size_t length);
    
    int process_content_further(tstring const& s, bool eof);
    protocol_listener& sink_;
    parse_mode mode_;
    
    size_t current_body_chunk_length;
    size_t current_body_chunk_readed;
    
    tinfra::lazy_byte_consumer<protocol_parser> dispatch_helper_;
};

} } // end namespace tinfra::http

#endif // tinfra_http_h_included_

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

