#include <tinfra/cmd.h>
#include <tinfra/fmt.h>
#include <tinfra/trace.h>
#include <tinfra/path.h>
#include <tinfra/subprocess.h>

#include <cassert>
#include <string>
#include <iostream>

#include <tinfra/generator.h>

#include "tinfra/shared_ptr.h"

using tinfra::shared_ptr;

enum pipe_status {
    success,
    not_ready,
    eof
};
    
template <typename OUT>
class source_base {
public:
    typedef OUT value_type;
    
    virtual ~source_base() {}
    virtual pipe_status read(value_type&) = 0;

    // implementation of generator_impl contract
    //inline bool fetch_next(value_type& v) { return (*this)(v); }
};

template<typename IN, typename OUT>
class pipe_base: public source_base<OUT> {
public:
    typedef IN  source_value_type;
    typedef OUT value_type;

    typedef source_base<source_value_type> source_type;
    virtual ~pipe_base() {}
    
    virtual pipe_status process(source_value_type const&, value_type& dest) = 0;
    
    virtual pipe_status read(value_type& dest)
    {        
        assert(source_impl_.get() != 0);
        while( true ) {
            source_value_type tmp;
            pipe_status p = source_impl_->read(tmp);
            if( p != success )
                return p;
            p = process(tmp, dest);
            if( p != not_ready )
                return p;
        }
    }
    void bind(shared_ptr<source_type> impl_) 
    {
        source_impl_ = impl_;
    }
private:
    shared_ptr<source_type> source_impl_;
};

template <typename OUT>
class source: public tinfra::generator_impl< source<OUT>, OUT > {
public:
    typedef OUT value_type;
    typedef source_base<OUT> impl_type;

    source(source<OUT> const& o): source_impl_(o.source_impl_) {}
        
    source(impl_type* ns): source_impl_(ns) {}
        
    template <typename T>
    source(shared_ptr<T> ns): source_impl_(ns) {}
        
    template <typename T>
    source(T const& impl): source_impl_(new T(impl)) {}
        
    pipe_status operator() (value_type& dest) {
        pipe_status r = source_impl_->read(dest);
        //TINFRA_TRACE_VAR(dest);
        return r;
    }
    
    shared_ptr< impl_type > impl_ptr() {
        return source_impl_;
    }
    
    // implementation of generator_impl contract
    //inline bool fetch_next(value_type& v) { return (*this)(v); }
    bool fetch_next(OUT& dest) {
        while( true) {
            pipe_status p = this->operator()(dest);
            switch( p  ) {
            case success: return true;
            case eof:      return false;
            case not_ready: continue;
            }
        }
    }
    template <typename PIPE>
    source< typename PIPE::value_type > operator | (PIPE const& p) const
    {
        typedef typename PIPE::value_type OUT;
        shared_ptr< PIPE > ptr( new PIPE(p) );
        ptr->bind(source_impl_);
        return source<OUT>(ptr);
    }
private:    
    shared_ptr< impl_type > source_impl_;
};



class stream_source: public source_base<char> {
public:
    stream_source(tinfra::io::stream* s): s_(s) {}
    
    pipe_status read(char& out) {
        char c;
        if( s_->read(&c, 1) == 0 )
            return eof;
        out = c;
        return success;
    }
private:
    tinfra::io::stream* s_;
};

class line_reader: public pipe_base<char, std::string> {
public:
    pipe_status process(char const& in, std::string& out) {
        out.append(1, in);
        if( in == '\n' )
            return success;
        else
            return not_ready;
    }
};

template <typename T>
source<typename T::value_type> make_source(T const& v)
{
    return source<typename T::value_type>(v);
}

class string_stripper: public pipe_base<std::string, std::string> {
public:
    pipe_status process(std::string const& in, std::string& out) {
        out = tinfra::strip(in);
        return success;
    }
};

class counter: public pipe_base<std::string, size_t> {
public:
    pipe_status process(std::string const& in, size_t& out) {
        out = in.size();
        return success;
    }
};

int pipe_main(int argc, char** argv)
{
    using std::auto_ptr;
    using std::string;
    
    using tinfra::io::stream;
    using tinfra::io::open_file;
    
    auto_ptr<stream> in( open_file("pipe.cpp", std::ios_base::in) );
    
    source<string> s = make_source(stream_source(in.get())) | line_reader() | string_stripper();
    
    int n = 0;
    for( source<string>::stl_iterator i = s.current(); i != s.end(); ++i ) {
        std::cout << ++n << ": " << *i << "\n";
    }
    return 0;
}

TINFRA_MAIN(pipe_main)
