#include <pcre.h>

#include <string>
#include <vector>

#include <iostream>
#include <stdexcept>

#include "tinfra/fmt.h"
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
        return do_match(0, str, length);
    }
    bool matches(match_result& result, const char* str, size_t length) const {
        return do_match(&result, str, length);
    }
    
    bool matches(const char* str) const {
        return do_match(0, str, std::strlen(str));
    }
    
    bool matches(match_result& result, const char* str) const  {
        return do_match(&result, str, std::strlen(str));
    }
private:
    void compile(const char* pattern, int options);
    
    bool do_match(match_result* result, const char* str, size_t length) const;
};

//
// implementation
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


bool regexp::do_match(match_result* result, const char* str, size_t length) const
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
    return true;
}


int regexp_pcre_main(int argc, char** argv)
{
    regexp re(argv[1]);
    std::string line;
    while( std::getline(std::cin, line) ) {
        if( re.matches(line.c_str(), line.size() ) ) { 
            std::cout << line << std::endl;
        }
    }
    return 0;    
}

TINFRA_MAIN(regexp_pcre_main);
