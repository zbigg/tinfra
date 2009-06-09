#include "http.h"

#include <tinfra/tstring.h>
#include <tinfra/trace.h>

#include <unittest++/UnitTest++.h>

#include <vector>
#include <utility>
#include <string>


SUITE(tinfra_http) {
    using std::make_pair;
    using std::vector;
    using std::pair;
    using std::string;
    
    struct mock_http_listener: public tinfra::http::protocol_listener {
        typedef tinfra::tstring tstring;
        
        virtual void request_line(
            tstring const& method, 
            tstring const& request_uri,
            tstring const& protocol_version)
        {
            
            this->method = method.str();
            this->request_uri = request_uri.str();
            this->protocol_version = protocol_version.str();
        }
        virtual void response_line(
            tstring const& protocol_version,
            tstring const& status,
            tstring const& status_string)
        {
            this->protocol_version = protocol_version.str();
            this->status = status.str();
            this->status_string = status_string.str();
        }
        
        virtual void header(
            tstring const& name, 
            tstring const& value)
        {
            TINFRA_TRACE_VAR(name);
            TINFRA_TRACE_VAR(value);
            headers.push_back(make_pair(name.str(), value.str()));
        }
        
        virtual void finished_headers()
        {
        }
        
        virtual void content(
            tstring const& data,
            bool last)
        {
            readed_contents.append(data.data(), data.size());
        }
        
        string method;
        string request_uri;
        string protocol_version;
        
        string status_string;
        string status;
        
        vector<pair<string,string> > headers;
        
        string readed_contents;
    };
    
    class buffer_parser_feeder {
    public:
        buffer_parser_feeder(tinfra::tstring const& buffer, size_t step_size=0):
            buffer_(buffer),
            index_(0),
            step_size_(0)
            
        {}
        
        void process(tinfra::parser& p)
        {
            size_t current_size = tinfra::tstring::npos;
            
            while( index_ < buffer_.size() ) {
                const tinfra::tstring current_value = buffer_.substr(index_, current_size);
                const int processed = p.process_input(current_value);
                assert(processed >= 0);
                //if( step_size_ != 0 ) {
                //    if( processed == 0 && current_size_ +
                //}
                 
                if( processed == 0 ) {
                    p.eof(current_value);
                    return;
                }
                
                index_ += processed;
                //if( step_size != 0 )
                //    current_size_ = step_size_;
            }
        }
    private:
        tinfra::tstring buffer_;
        size_t          index_;
        size_t          step_size_;
    };
    
    using tinfra::http::protocol_listener;
    using tinfra::http::protocol_parser;
    
    void feed(protocol_parser::parse_mode pm, protocol_listener& pl,
        std::string const& message)
    {
        protocol_parser parser(pl, pm);
        buffer_parser_feeder feeder(message);
        buffer_parser_feeder feeder1(message, 1);
        
        
        feeder.process(parser);
    }
    
    TEST(request_simple) {
        
        mock_http_listener result;
        feed(protocol_parser::SERVER, result, 
            "GET / HTTP/1.0\r\n"
            "\r\n");
        
        CHECK_EQUAL("GET",      result.method);
        CHECK_EQUAL("/",        result.request_uri);
        CHECK_EQUAL("HTTP/1.0", result.protocol_version);
        CHECK_EQUAL(0,          result.headers.size());
        CHECK_EQUAL(0,          result.readed_contents.size());
    }
    
    TEST(request_complex_with_content) {
        
        mock_http_listener result;
        feed(protocol_parser::SERVER, result, 
            "GET /quite_strane%20uri HTTP/1.1\r\n"
            "Content-length: 32\r\n"
            "Host: foo.bar\r\n"
            "\r\n"
            "0123456789abcdef"
            "0123456789ABCDEF"
            );
        
        CHECK_EQUAL("GET",      result.method);
        CHECK_EQUAL("/quite_strane%20uri",        result.request_uri);
        CHECK_EQUAL("HTTP/1.1", result.protocol_version);
        CHECK_EQUAL(2,          result.headers.size());
        
        CHECK_EQUAL("Content-length",  result.headers[0].first);
        CHECK_EQUAL("32",              result.headers[0].second);
        
        CHECK_EQUAL("Host",            result.headers[1].first);
        CHECK_EQUAL("foo.bar",         result.headers[1].second);
        
        CHECK_EQUAL(32,                result.readed_contents.size());
        CHECK_EQUAL("0123456789abcdef"
                    "0123456789ABCDEF",result.readed_contents);
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

