#include <stdexcept>

template<typename R, typename T>
class interruptible {
public:
    typedef R (interruptible::*step_method)(T const& e);
    
    interruptible(step_method m = 0):
        next_method_(m),
        again_(false),
        finished_(false)
    {
    }
    
    bool process(T const& input)
    {
        if( finished_ ) {
            throw std::logic_error("already finished");
        }
        step_method current_method = next_method_;
        next_method_ = 0;
        again_ = false;
        
        R output = call(current_method, input);
        
        if( again_ ) {
            next_method_ = current_method;
        } else if( !next_method_ ) {
            finished_  = true;
        }
        return output;
    }
    
    bool is_finished() const {
        return finished_;
    }
protected:
    
    R call(step_method m, T const& a)
    {
        return (this->*m)(a);
    }
    void next(step_method s) {
        next_method_ = s;
    }
    
    template <typename F>
    step_method make_step_method(R (F::*m)(T const&)) {
        return reinterpret_cast<step_method>(m);
    }
    void again() {
        again_ = true;
    }
    
    void finish() {
        // nothing needed
    }
private:
    step_method next_method_;
    bool        again_;
    bool        finished_;
};

#include <tinfra/tstring.h>
#include <string>

using tinfra::tstring;
using std::string;

class lazy_protocol: public interruptible<int, tstring> {
protected:
    
    void wait_for_bytes(size_t count, step_method method)
    {
        waiter_count_ = count;
        
        waiter_method_ = method;        
        next(make_step_method(&lazy_protocol::maybe_have_enough_bytes));
    }
    
    void wait_for_delimiter(tstring const& delim, step_method method)
    {
        waiter_delim_.assign(delim.data(), delim.size());
        waiter_method_ = method;
        next(make_step_method(&lazy_protocol::maybe_have_delim));
    }
private:
    step_method waiter_method_;
    size_t      waiter_count_;
    string      waiter_delim_;

    int maybe_have_delim(tstring const& input)
    {
        size_t pos =  input.find_first_of(waiter_delim_.data(), waiter_delim_.size());
        if( pos == tstring::npos ) {
            again();
            return 0;
        }
        return call(waiter_method_, tstring(input.data(), pos));
    }
    
    int maybe_have_enough_bytes(tstring const& input)
    {
        if( input.size() < waiter_count_ ) {
            again();
            return 0;
        }
        
        return call(waiter_method_, tstring(input.data(), waiter_count_));
    }
    
};

struct http_request_processor {
    virtual void request_line(
        tstring const& method, 
        tstring const& request_uri,
        tstring const& proto_version) = 0;
    virtual void header(
        tstring const& name, 
        tstring const& value,
        bool last) = 0;
    virtual void content(
        tstring const& data,
        bool last) = 0;
};

class http_request_parser: public lazy_protocol  {
    
    http_request_parser(http_request_processor& p) {
        : processor(p)
    {
        setup_initial_state();
    }

    bool is_keep_alive() const { return false; }
    
private:
    http_request_processor& processor;

    static regexp http_11_request_line_re;
    static regexp http_10_request_line_re;

    int request_line(tstring const& s) {        
        wait_for_delimiter("\r\n", 
            make_step_method(&http_request_parser::header_line));
                
        // TODO: store/consume request-line        
        return s.size();
    }
    
    int header_line(tstring const& s) {
        if( s.size() > 2 ) {
            // TODO: store/consume headers
        } else if( s.size() == 2 ) {
            // headers ENDED
            // somehow find content-length
            requested_content_length = 100;
            readed_content_length = 0;
            
            setup_content_retrieval();
        }
        return s.size();
    }
    
    void setup_initial_state()
    {
        wait_for_delimiter("\r\n",
            make_step_method(&http_request_parser::request_line));
    }
    
    void setup_content_retrieval()
    {
        size_t remaining_size = requested_content_length - readed_content_length;
        if( remaining_size > 0 ) {
            next(make_step_method(&http_request_parser::content_bytes));
        } else if( is_keep_alive() ) {
            setup_initial_state();
        } else {
            finish();
        }
    }
    int content_bytes(tstring const& s) {
        size_t remaining_size = requested_content_length - readed_content_length;        
        size_t current_size = std::min(remaining_size, s.size());
        
        // TODO: consume/store current_size bytes
        
        readed_content_length += current_size;
        setup_content_retrieval();
        return current_size;
    }

    size_t requested_content_length;
    size_t readed_content_length;
};

int main()
{
    return 0;
}
