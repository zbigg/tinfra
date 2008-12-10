//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_regex_h__
#define __tinfra_regex_h__

#include "tinfra/platform.h"

#ifdef HAVE_PCRE
#define TINFRA_REGEX_PCRE 1
#else
#endif

#include <string>
#include <vector>
#include <cassert>

#include "tinfra/tstring.h"
#include "tinfra/tinfra_lex.h"

namespace tinfra {

struct match_result_processor {
    virtual void prepare(int groups) = 0;
    virtual void match_group(int group_no, const char* str, size_t pos, size_t len) = 0;
    
    virtual ~match_result_processor() {}
};

struct std_match_result: public match_result_processor {
    void prepare(int groups);
    void match_group(int group_no, const char* str, size_t pos, size_t len);
    
    std::vector<std::string> groups;
};

template <int N>
struct static_tstring_match_result: public match_result_processor {
    void prepare(int groups) {
        assert(groups == N);
    }
    void match_group(int group_no, const char* str, size_t pos, size_t len) {
        const char* begin = str+pos;
        groups[group_no] = tstring(begin, len);
    }
    
    tstring groups[N];
};

class regexp {
    
#ifdef TINFRA_REGEX_PCRE
    void*  re_;
    void*  extra_;
#endif

    size_t patterns_count_;
public:
    //typedef std_match_result match_result; // deprecated
    
    regexp(const char* pattern);
    ~regexp();
    
    bool matches(tstring const& str) const {
        return do_match(0, str.data(), str.size(), 0);
    }
    bool matches(tstring const& str, match_result_processor& result) const {
        return do_match(&result, str.data(), str.size(), 0);
    }
    
    bool matches(const char* str, size_t length) const {
        return do_match(0, str, length, 0);
    }
    bool matches(const char* str, size_t length, match_result_processor& result) const {
        return do_match(&result, str, length, 0);
    }
    
    bool matches(const char* str) const {
        return do_match(0, str, std::strlen(str),0 );
    }
    
    bool matches(const char* str, match_result_processor& result) const  {
        return do_match(&result, str, std::strlen(str), 0 );
    }
    
    bool do_match(match_result_processor* result, const char* str, size_t length, size_t* finish_offset) const;
    
    size_t groups_count() const { return patterns_count_; }
    
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
    std_match_result match_;
public:
    matcher(regexp const& re, const char* str, size_t length);
    matcher(regexp const& re, const char* str);
    
    std_match_result const& next();
    
    bool has_next();

private:
    void try_match();
};

//
// scanner
//

class scanner {
    std_match_result match_;
    size_t  current_param_;
    bool    have_match_;
public:
    scanner(regexp const& re, const char* str):  
        current_param_(0),
        have_match_(re.matches(str, match_))
    {
    }
    scanner(regexp const& re, const char* str, size_t length):  
        current_param_(0),
        have_match_(re.matches(str, length, match_))
    {
    }
    
    bool matches() const { return have_match_; }
    
    operator bool() const { return matches(); }
    
    template <typename T>
    scanner& parse(T& value);
    
    template <typename T>
    scanner& operator >>(T& value) { return parse(value); }
    
    template <typename T>
    scanner& operator %(T& value) { return parse(value); }
private:
    bool check_and_forward();
};

template <typename T>
scanner& scanner::parse(T& value) {
    if( check_and_forward() ) {
        tinfra::from_string<T>(match_.groups[current_param_], value);
    }
    return *this;
}

}  // end namespace tinfra

#endif // __tinfra_regex_h__
