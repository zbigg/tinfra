//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_fmt_h_included
#define tinfra_fmt_h_included

#include "tinfra/platform.h"

#include "tinfra/tstring.h"
#include "stream.h" // for tinfra::output_stream

#include <string>  // for std::string
#include <iosfwd>  // for std::ostream
#include <sstream> // for std::ostringstream 
#include <stdexcept> // for std::logic_error

namespace tinfra {
    
/// simple_fmt
/// formats everyting as string using tinfra::to_string()
/// Use case:
///     fmt("Hello %s. Nice to %s you. Count %i") << "zbyszek" << "opryszek" << 2;
///     fmt("Hello %s. Nice to %s you. Count %i") % "zbyszek" % "opryszek" % 2;
///

class format_exception: public std::logic_error {
public:
    format_exception(const std::string& message): std::logic_error("format exception: " + message) {}
};

class basic_fmt {
public:
    basic_fmt(std::streambuf* buf, tstring const& format):
        buf_(buf),
        fmt_(format), 
        pos_(0) 
    {}
    
    template <typename T>
    basic_fmt& push(T const& value) {
        std::ostream formatter(this->buf_);
        const std::size_t cmd_pos = check_command(formatter);
        formatter << value;
        this->pos_= cmd_pos;
        return *this;
    }
#ifdef TINFRA_HAS_VARIADIC_TEMPLATES
    basic_fmt& push() {
        return *this;
    }
    template <typename T, typename... Args>
    basic_fmt& push(T const& value, Args... args) {
        push(value);
        push(args...);
        return *this;
    }
#endif
    
    template <typename T>
    basic_fmt& operator <<(T const& value) { return push(value); }
    
    template <typename T>
    basic_fmt& operator %(T const& value) { return push(value); }
    
    void reset();

    void flush() { realize(); }
    
private:

    std::size_t check_command(std::ostream&);
    void realize();

    std::streambuf* buf_; // where write to ?
    std::string     fmt_; // format string
    std::size_t     pos_; // where we are in parsing format ?
};


class stringbuf_fmt {
    std::stringbuf the_buf_;
    basic_fmt      delegate_;
public:
    stringbuf_fmt(tstring const& format):
        delegate_(&the_buf_, format)
    {
    }

#ifdef TINFRA_HAS_VARIADIC_TEMPLATES
    template <typename... Args>
    stringbuf_fmt& push(Args... args) {
        delegate_.push(args...);
        return *this;
    }
#else
    
    template <typename T>
    stringbuf_fmt& push(T const& value) { delegate_.push(value); return *this; }
#endif
    
    template <typename T>
    stringbuf_fmt& operator <<(T const& value) { return push(value); }
    
    template <typename T>
    stringbuf_fmt& operator %(T const& value) { return push(value); }
    
    std::string str() {
        delegate_.flush();
        return the_buf_.str();
    }
    
    operator std::string () {
        return str();
    }
    
    /*
        ok, now this is shit and shouldn't be supported
        if one wants "in-memory" and optimized formatter
        then he should use basic_fmt and pass "own"
        streambuf
    */
private:
    std::string tmp_output_;
public:
    operator tinfra::tstring() {
        tmp_output_ = this->str();
        return tinfra::tstring(tmp_output_);
    }
    
    operator const char*() {
        tmp_output_ = this->str();
        return tmp_output_.c_str();
    }
    
    /*
    operator std::string const&() {
        tmp_output_ = this->str();
        return tmp_output_;
    }
    */
};

std::ostream& operator << (std::ostream& out, stringbuf_fmt& fmt);

///
/// The default formatter supported by tinfra.
///
typedef stringbuf_fmt fmt;

//
// printf & sprintf like wrappers for basic_fmt
//

///
/// template <typename OUT, typename... Args>
/// void tprintf(OUT& out, Args... args);
///
///
/// NOTE, they are available also without variadic template support, but
/// implemented "by hand" below

#ifdef TINFRA_HAS_VARIADIC_TEMPLATES
template < typename... Args>
void tprintf(std::ostream& out, tinfra::tstring const& fmt, Args... args);

template < typename... Args>
std::string tsprintf(tstring const& fmt, Args ... args);
#endif


//
// tsprintf and tprintf implementation
//
#ifdef TINFRA_HAS_VARIADIC_TEMPLATES
template <typename... Args>
void tprintf(std::ostream& out, tinfra::tstring const& fmt, Args... args)
{
    basic_fmt F(out.rdbuf(), fmt);
    F.push(args...);
    F.flush();
}

template <typename ... Args>
std::string tsprintf(tstring const& fmt, Args ... args) {
    stringbuf_fmt F(fmt);
    F.push(args...);
    return F.str();
}


#else // no TINFRA_HAS_VARIADIC_TEMPLATES

// tprintf(std::ostream&...)
//

inline void tprintf(std::ostream& out, tstring const& fmt) {
    basic_fmt F(out.rdbuf(), fmt);
    F.flush();
}

template <typename T1>
inline void tprintf(std::ostream& out, tstring const& fmt, T1 const& v1) {
    basic_fmt F(out.rdbuf(), fmt);
    F << v1;
    F.flush();
}

template <typename T1, typename T2>
inline void tprintf(std::ostream& out, tstring const& fmt, T1 const& v1, T2 const& v2) {
    basic_fmt F(out.rdbuf(), fmt);
    F << v1 << v2;
    F.flush();
}

template <typename T1, typename T2, typename T3>
inline void tprintf(std::ostream& out, tstring const& fmt, T1 const& v1, T2 const& v2, T3 const& v3) {
    basic_fmt F(out.rdbuf(), fmt);
    F << v1 << v2 << v3;
    F.flush();
}

template <typename T1, typename T2, typename T3, typename T4>
inline void tprintf(std::ostream& out, tstring const& fmt, T1 const& v1, T2 const& v2, T3 const& v3, T4 const& v4) {
    basic_fmt F(out.rdbuf(), fmt);
    F << v1 << v2  << v3 << v4;
    F.flush();
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
inline void tprintf(std::ostream& out, tstring const& fmt, T1 const& v1, T2 const& v2, T3 const& v3, T4 const& v4, T5 const& v5 ) {
    basic_fmt F(out.rdbuf(), fmt);
    F << v1 << v2  << v3 << v4 << v5;
    F.flush();
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline void tprintf(std::ostream& out, tstring const& fmt, T1 const& v1, T2 const& v2, T3 const& v3, T4 const& v4, T5 const& v5, T6 const& v6) {
    basic_fmt F(out.rdbuf(), fmt);
    F << v1 << v2  << v3 << v4 << v5 << v6;
    F.flush();
}

// tprintf(tinfra::output_string&...)
//

inline void tprintf(tinfra::output_stream& out, tstring const& fmt) {
    stringbuf_fmt F(fmt);
    out.write(F);
}

template <typename T1>
inline void tprintf(tinfra::output_stream& out, tstring const& fmt, T1 const& v1) {
    stringbuf_fmt F(fmt);
    F << v1;
    out.write(F);
}

template <typename T1, typename T2>
inline void tprintf(tinfra::output_stream& out, tstring const& fmt, T1 const& v1, T2 const& v2) {
    stringbuf_fmt F(fmt);
    F << v1 << v2;
    out.write(F);
}

template <typename T1, typename T2, typename T3>
inline void tprintf(tinfra::output_stream& out, tstring const& fmt, T1 const& v1, T2 const& v2, T3 const& v3) {
    stringbuf_fmt F(fmt);
    F << v1 << v2 << v3;
    out.write(F);
}

template <typename T1, typename T2, typename T3, typename T4>
inline void tprintf(tinfra::output_stream& out, tstring const& fmt, T1 const& v1, T2 const& v2, T3 const& v3, T4 const& v4) {
    stringbuf_fmt F(fmt);
    F << v1 << v2  << v3 << v4;
    out.write(F);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
inline void tprintf(tinfra::output_stream& out, tstring const& fmt, T1 const& v1, T2 const& v2, T3 const& v3, T4 const& v4, T5 const& v5 ) {
    stringbuf_fmt F(fmt);
    F << v1 << v2  << v3 << v4 << v5;
    out.write(F);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline void tprintf(tinfra::output_stream& out, tstring const& fmt, T1 const& v1, T2 const& v2, T3 const& v3, T4 const& v4, T5 const& v5, T6 const& v6) {
    stringbuf_fmt F(fmt);
    F << v1 << v2  << v3 << v4 << v5 << v6;
    out.write(F);
}

// tsprintf
// 
inline std::string tsprintf(tstring const& fmt) {
    stringbuf_fmt F(fmt);
    return F.str();
}

template <typename T1>
inline std::string tsprintf(tstring const& fmt, T1 const& v1) {
    stringbuf_fmt F(fmt);
    F << v1;
    return F.str();
}

template <typename T1, typename T2>
inline std::string tsprintf(tstring const& fmt, T1 const& v1, T2 const& v2) {
    stringbuf_fmt F(fmt);
    F << v1 << v2;
    return F.str();
}

template <typename T1, typename T2, typename T3>
inline std::string tsprintf( tstring const& fmt, T1 const& v1, T2 const& v2, T3 const& v3) {
    stringbuf_fmt F(fmt);
    F << v1 << v2  << v3;
    return F.str(); 
}

template <typename T1, typename T2, typename T3, typename T4>
inline std::string tsprintf(tstring const& fmt, T1 const& v1, T2 const& v2, T3 const& v3, T4 const& v4) {
    stringbuf_fmt F(fmt);
    F << v1 << v2  << v3 << v4;
    return F.str();
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
inline std::string tsprintf(tstring const& fmt, T1 const& v1, T2 const& v2, T3 const& v3, T4 const& v4, T5 const& v5 ) {
    stringbuf_fmt F(fmt);
    F << v1 << v2  << v3 << v4 << v5;
    return F.str();
}



template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline std::string tsprintf(tstring const& fmt, T1 const& v1, T2 const& v2, T3 const& v3, T4 const& v4, T5 const& v5, T6 const& v6) {
    stringbuf_fmt F(fmt);
    F << v1 << v2  << v3 << v4 << v5 << v6;
    return F.str();
}


#endif // !TINFRA_HAS_VARIADIC_TEMPLATES

} // end namespace tinfra

#endif

