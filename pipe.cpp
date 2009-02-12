#include <tinfra/cmd.h>
#include <tinfra/fmt.h>
#include <tinfra/trace.h>
#include <tinfra/path.h>
#include <tinfra/subprocess.h>

#include <cassert>
#include <string>

#include <tinfra/generator.h>

#include "smart_ptr.h"


typedef bool pipe_status;

template<typename IN, typename OUT> class pipe;
    
template <typename OUT>
class source_base: public tinfra::generator_impl<source_base<OUT>, OUT> {
public:
    typedef OUT value_type;
    
    virtual ~input_base() {}
    virtual pipe_status operator()(value_type&) = 0;

    // implementation of generator_impl contract
    inline bool fetch_next(value_type& v) { return (*this)(v); }
    
    template <typename PIPE>
    PIPE operator | (PIPE const&);
};

template<typename IN, typename OUT>
class pipe: public source_base<OUT> {
public:
    typedef IN  source_value_type;
    typedef OUT value_type;

    typedef input_base<source_value_type> source_type;
    
    template <typename T>
    pipe bind_source(shared_ptr<T> i) {
        source_ = i;
        return *this;
    }
    
    virtual pipe_status operator()(value_type& dest) = 0;
        source_value_type tmp;
        pipe_status ps = source()(tmp);
        if( !ps )
            return ps;
        
        return (*this)(tmp, dest);
        
    }
    virtual pipe_status operator()(source_value_type const&, value_type& dest) = 0;
    
private:
    source_type& source() { 
        assert(source.get());
        return * (source_.get()) ; 
    }
    shared_ptr<source_type> source_;
};

template <typename OUT, typename PIPE>
PIPE source_base<OUT>::operator | (PIPE const&);
{
    return PIPE(sb).bind_source(*this);
}

class stream_source: public source_base<tinfra::io::stream*> {
public:
    stream_source(tinfra::io::stream* s): s_(s) {}
    
    pipe_status operator(tinfra::io::stream*& out) {
        out = s_;
    }
    
    tinfra::io::stream* s_;
};

class line_reader: public pipe<tinfra::io::stream*, std::string> {
public:
    
    pipe_status operator(tinfra::io::stream* const& out, std::string& out) {
        size_t eol_pos;
        while( true ) {
            eol_pos =  buffer.find_first_of('\n');
            if( eol_pos != string::npos ) {
                break;
            }
            if( !readsome() ) {
                if( buffer.size() > 0 ) {
                    out = buffer;
                    buffer.clear();
                    return true;
                } else {
                    return false;
                }
            }
        }
        
        out.assign(buffer.data(), eol_pos+1);
        out.erase(0, eol_pos+1);
        return true;
    }
private:
    typedef std::string string;
    string buffer;
};


template <typename T>
class string_stripper: public pipe<std::string, std::string> {
public:
    pipe_status operator(std::string const& in, std::string& out) {
        out = tinfra::strip(in);
    }
};
*/
int pipe_main(int argc, char** argv)
{
    tinfra::shared_ptr<std::string>  sptr;
    sptr a(new std::string("anc"));
    
    sptr b(a);
    sptr c(a);
    sptr d;
    d = b;
    d = a;
    return 0;
}

TINFRA_MAIN(pipe_main);
