#include <pcre.h>

#include <string>
#include <vector>

#include <iostream>
#include <stdexcept>

#include "tinfra/fmt.h"
#include "tinfra/string.h"
#include "tinfra/cmd.h"

class regexp {
    ::pcre*       re_;
    ::pcre_extra* extra_;
    size_t     patterns_count_;
public:
    typedef std::vector<std::string> match_result;
    
    regexp(const char* pattern, int options = 0);
    ~regexp();
    
    bool matches(const char* str, size_t length) const {
        return do_match(0, str, length, 0);
    }
    bool matches(const char* str, size_t length, match_result& result) const {
        return do_match(&result, str, length, 0);
    }
    
    bool matches(const char* str) const {
        return do_match(0, str, std::strlen(str),0 );
    }
    
    bool matches(const char* str, match_result& result) const  {
        return do_match(&result, str, std::strlen(str), 0 );
    }
    
    bool do_match(match_result* result, const char* str, size_t length, size_t* finish_offset) const;
    
private:
    void compile(const char* pattern, int options);
};

class matcher {
    regexp const& re_;
    const char* str_;
    size_t length_;
    size_t position_;
    bool have_result_;
    bool have_match_;
    regexp::match_result match_;
public:
    matcher(regexp const& re, const char* str, size_t length);
    matcher(regexp const& re, const char* str);
    
    regexp::match_result const& next();
    
    bool has_next();

private:
    void try_match();
};

//
// implementation
//

//
// regexp
//
using tinfra::fmt;

regexp::regexp(const char* pattern, int options):
    re_(0),
    extra_(0),
    patterns_count_(0)
{
    compile(pattern, options);
}

regexp::~regexp()
{
    if( extra_ )
        pcre_free(extra_);
    if( re_ )
        pcre_free(re_);
}

void regexp::compile(const char* pattern, int options)
{
    const char* err_ptr = 0;
    int         err_offset;
    
    options = 0; //PCRE_NEWLINE_ANY;
    
    re_ = pcre_compile(pattern, options, &err_ptr, &err_offset,0);
    if( re_ == 0 ) {
        if( err_ptr == 0 ) 
            err_ptr = "unknown error";
        throw std::logic_error(fmt("bad regular expression '%s': %s") % pattern % err_ptr);
    }
    extra_ = pcre_study(re_, 0,  &err_ptr);
    if( extra_ == 0 && err_ptr != 0 ) {
        throw std::logic_error(fmt("pcre_study failed: %s") % err_ptr);
    }
    
    pcre_fullinfo(re_, extra_, PCRE_INFO_CAPTURECOUNT, &patterns_count_);
}


bool regexp::do_match(match_result* result, const char* str, size_t length, size_t* finish_offset) const
{
    const int offsets_size = (patterns_count_+1)*3; // see manual, pcre_exec def
    int offsets[offsets_size]; 
    int options = 0;
    int rc = pcre_exec(re_, extra_, str, length, 0, options, 
                       offsets, offsets_size);
    if( rc == -1 )
        return false;
    
    if( rc < 0 ) {
        const char* err_ptr = "unknown error";
        throw std::logic_error(fmt("PCRE match failed: %s") % err_ptr);
    }
    if( result != 0 ) {
        match_result& r = *result;
        r.resize(patterns_count_+1);
        for( size_t i = 0; i <= patterns_count_; ++i ) {
            const char* p = str + offsets[i*2];
            const char* e = str + offsets[i*2 +1];
            const size_t len = e-p;
            r[i].assign(p, len);
        }
    }
    if( finish_offset != 0 ) {
        *finish_offset = offsets[1];
    }
    return true;
}

//
// matcher
//

matcher::matcher(regexp const& re, const char* str, size_t length):
    re_(re),
    str_(str),
    length_(length),
    position_(0),
    have_result_(false),
    have_match_(false)
{}

matcher::matcher(regexp const& re, const char* str):
    re_(re),
    str_(str),
    length_(std::strlen(str)),
    position_(0),
    have_result_(false),
    have_match_(false)
{}

regexp::match_result const& matcher::next() {
    if( ! have_result_ ) 
        try_match();
    if( have_match_ ) {
        have_result_ = false;
        return match_;
    } else {
        throw std::runtime_error("no more matches");
    }
}

bool matcher::has_next()
{
    if( !have_result_ ) 
        try_match();
    return have_match_;
}


void matcher::try_match()
{
    have_result_ = true;
    if( position_ == length_ ) {
        have_match_ = false;
    } else {
        size_t match_end;
        have_match_ = re_.do_match(&match_, 
                                   str_ + position_, length_ - position_,
                                   &match_end);
        position_ = position_ + match_end;
    }
}

//
// sample program, proof of concept
//
int regexp_pcre_main(int argc, char** argv)
{
    regexp re(argv[1]);
    std::string line;
    
    while( std::getline(std::cin, line) ) {
        tinfra::strip_inplace(line);
        for(matcher m(re, line.c_str(), line.size()); m.has_next(); ) {
            regexp::match_result const& match = m.next();
            std::cout << match[0] << std::endl;
        }
    }
    return 0;    
}

TINFRA_MAIN(regexp_pcre_main);
