//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include <iostream>
#include <stdexcept>

#include "tinfra/fmt.h"
#include "tinfra/regexp.h"

#ifdef TINFRA_REGEX_POSIX

#include <regex.h>

namespace tinfra {
static size_t impl_get_groups_count(regex_t* re, const char* pattern)
{
#ifdef __GLIBC__
        return re->re_nsub;
#endif
#if 1   // BSD documentation also states this
        return re->re_nsub;
#endif
        return -1;
}
regexp::regexp(const char* pattern):
    patterns_count_(-1)
{
    compile(pattern, 0);
    patterns_count_ = impl_get_groups_count(re_, pattern);
}

regexp::~regexp()
{
}

namespace {
    void throw_reg_error(int ret_code, regex_t& re)
    {
        char buf[1024];
        regerror(ret_code, &re, buf, sizeof(buf));
        throw std::logic_error(tinfra::fmt("regcomp failed: %s") % buf);
    }
}
void regexp::compile(const char* pattern, int options)
{
    int cflags = REG_EXTENDED;
    int r = regcomp(&re_, pattern, cflags);
    if( r != 0 ) {
        throw_reg_error(r, re_); 
    }
}

bool regexp::do_match(match_result_processor* result, tstring const& str, size_t* finish_offset) const
{
    string_pool temporary_string_pool;
    const char* c_str = str.c_str(temporary_string_pool);
    const size_t NMATCH = patterns_count_;
    regmatch_t match[NMATCH];
    const int eflags = 0;
    int r = regexec(re_, null_terminated_str, NMATCH, match, eflags);
    if( r == REG_NOMATCH )
        return false;
    if( r != 0 ) {
        throw_reg_error(r, re_);
    }
    result->prepare(NMATCH);
    for( size_t i = 0; i < NMATCH; ++i ) {
        regmatch_t const& im = match[i];
        const size_t match_pos = im.rm_so;
        const size_t match_len  = im.rm_eo - im.rm_so;
        result->match_group(i, str.data(), match_pos, match_len);
    }
    return true;
}

#endif // TINFRA_REGEX_POSIX


} // end namespace tinfra

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

