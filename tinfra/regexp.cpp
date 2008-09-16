//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <iostream>
#include <stdexcept>

#include "tinfra/fmt.h"

#include "regexp.h"

//
// regexp
//

namespace tinfra {

#ifdef TINFRA_REGEX_PCRE
regexp::regexp(const char* pattern):

    re_(0),
    extra_(0),
    patterns_count_(0)
{
    compile(pattern, 0);
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
    
    re_ = pcre_compile(pattern, options, &err_ptr, &err_offset, 0);
    if( re_ == 0 ) {
        if( err_ptr == 0 ) 
            err_ptr = "unknown error";
        throw std::logic_error(fmt("bad regular expression '%s': %s") % pattern % err_ptr);
    }
    extra_ = pcre_study(re_, 0,  &err_ptr);
    if( extra_ == 0 && err_ptr != 0 ) {
        // TODO: this should be abort because according to pcreapi manual
        //       it shouldn't fail with fresh and correct re_
        throw std::runtime_error(fmt("pcre_study failed: %s") % err_ptr);
    }
    {
        int cc = 0;
        int rc = pcre_fullinfo(re_, extra_, PCRE_INFO_CAPTURECOUNT, &cc);
        if( rc != 0 ) {
            // TODO: this should be abort because according to pcreapi manual
            //       it shouldn't fail with fresh and correct re_
            throw std::runtime_error(fmt("pcre_fullinfo(PCRE_INFO_CAPTURECOUNT) failed: %i") % rc);
        }
        patterns_count_ = cc;
    }
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

#endif // TINFRA_REGEX_PCRE

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
// scanner
//

bool scanner::check_and_forward()
{
    if( !have_match_ ) 
        return false; 
    current_param_++;
    if( current_param_ == match_.size() ) {
        throw std::logic_error("scanner: too many arguments");
    }
    return true;
}

} // end namespace tinfra

