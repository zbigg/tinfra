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

#ifdef TINFRA_REGEX_PCRE
#include <pcre.h>

namespace tinfra {

#define TT_PCRE(a)        (pcre*)(a)
#define TT_PCRE_EXTRA(a)  (pcre_extra*)(a)

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
    	pcre_free(TT_PCRE_EXTRA(extra_));
    if( re_ )
        pcre_free(TT_PCRE(re_));
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
    extra_ = pcre_study(TT_PCRE(re_), 0,  &err_ptr);
    if( extra_ == 0 && err_ptr != 0 ) {
        // TODO: this should be abort because according to pcreapi manual
        //       it shouldn't fail with fresh and correct re_
        throw std::runtime_error(fmt("pcre_study failed: %s") % err_ptr);
    }
    {
        int cc = 0;
        int rc = pcre_fullinfo(TT_PCRE(re_), TT_PCRE_EXTRA(extra_), PCRE_INFO_CAPTURECOUNT, &cc);
        if( rc != 0 ) {
            // TODO: this should be abort because according to pcreapi manual
            //       it shouldn't fail with fresh and correct re_
            throw std::runtime_error(fmt("pcre_fullinfo(PCRE_INFO_CAPTURECOUNT) failed: %i") % rc);
        }
        patterns_count_ = cc;
    }
}


bool regexp::do_match(match_result_processor* result, const char* str, size_t length, size_t* finish_offset) const
{
    const int offsets_size = (patterns_count_+1)*3; // see manual, pcre_exec def
    std::vector<int> offsets(offsets_size);
    int options = 0;
    int rc = pcre_exec(TT_PCRE(re_), TT_PCRE_EXTRA(extra_), 
	               str, length, 0, options, 
                       &(offsets[0]), offsets_size);
    if( rc == -1 )
        return false;
    
    if( rc < 0 ) {
        const char* err_ptr = "unknown error";
        throw std::logic_error(fmt("PCRE match failed: %s") % err_ptr);
    }
    if( result != 0 ) {
        result->prepare(groups_count()+1);
        for( size_t i = 0; i <= patterns_count_; ++i ) {
            const size_t p = offsets[i*2];
            const size_t e = offsets[i*2 +1];
            const size_t len = e-p;
            result->match_group(i, str, p, len);
        }
    }
    if( finish_offset != 0 ) {
        *finish_offset = offsets[1];
    }
    return true;
}

#endif // TINFRA_REGEX_PCRE


} // end namespace tinfra

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

