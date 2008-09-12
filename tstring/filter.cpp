#include <pcre.h>

#include <string>
#include <vector>

#include <iostream>
#include <iterator>
#include <stdexcept>

#include "tinfra/fmt.h"
#include "tinfra/string.h"
#include "tinfra/cmd.h"

class regexp {
    ::pcre*       re_;
    ::pcre_extra* extra_;
    size_t     patterns_count_;
    
public:
    struct match_entry {               
        std::string str() const { 
            return std::string(begin(), size()); 
        }

        match_entry(): _begin(0), _end(0) {}  
        match_entry(const char* begin, const char* end): 
            _begin(begin), 
            _end(end) 
        {}
        
        const char* begin() const { return _begin; }
        const char* end() const { return _end; }
        
        size_t      size() const { return _end - _begin; }
    private:
        const char* _begin;
        const char* _end;
    };
    
    typedef std::vector<match_entry> match_result;
    
    regexp(const char* pattern);
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
    regexp(regexp const& o) {}
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
            r[i] = match_entry(p, e);
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
// scanner
//

class scanner {
    regexp::match_result match_;
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
    scanner& push(T& value) {
        if( !have_match_ ) 
            return *this;
        current_param_++;
        if( current_param_ == match_.size() ) {
            throw std::logic_error("scanner: too many arguments");
        }
        tinfra::from_string<T>(match_[current_param_], value);        
        return *this;
    }
    
    template <typename T>
    scanner& operator <<(T& value) { return push(value); }
    
    template <typename T>
    scanner& operator %(T& value) { return push(value); }
};

//
// sample program, proof of concept
//

#include <algorithm>

#include "tinfra/tstring.h"

using std::string;
using tinfra::tstring;

//typedef std::vector<string> string_list;
typedef std::vector<tstring*> string_ref_list;

using tinfra::fmt;

struct rule {
    regexp       re;
    //string_list actions;
    
    rule(const char* re): re(re) {}
};

struct rope {
    string_ref_list elements;
};

typedef std::map<string, tstring*> var_mapping;

std::ostream& operator << (std::ostream& out, rope const& r)
{
    for(string_ref_list::const_iterator i = r.elements.begin(); i !=  r.elements.end(); ++i ) {
        out << **i;
    }
    return out;
}

#include "pool.h"


void process_line(std::ostream& out, string const& input, rule const& rule)
{
    static tinfra::pool<tstring> string_pool(50);
    static tinfra::byte_pool byte_pool(1024);
    
    regexp::match_result r;
    if( !rule.re.matches(input.c_str(), input.size(), r) ) {
        std::cout << input << std::endl;
        return;
    }
    
    const char* begin = input.c_str();
    const char* end = begin + input.size();
    const char* current_pos = begin;
    rope all;
    var_mapping context;
    
    for( size_t i =1; i < r.size(); ++i ) {
        regexp::match_entry const& me = r[i];
        tstring* e = string_pool.construct(me.begin(), me.size());
        tstring* p = string_pool.construct(current_pos, me.begin() - current_pos);
        
        context[fmt("%i") % i] = e;
        context[fmt("pre%i") % i] = p;
        
        all.elements.push_back(p);
        all.elements.push_back(e);
        
        if( i > 1 ) { 
            //context[fmt("post%i") % (i-1)] = p;
        }
        current_pos = me.end();
    }
    
    tstring* last = string_pool.construct(current_pos, end-current_pos);
    context[fmt("post%i") % r.size()] = last;
    all.elements.push_back(last);

    //process_var_mapping(var_mapping, rule);

    // TODO: write rules engine:
    //  $1=red($1)
    //  suffix(timestamp)
    //  erase(0-4)  - erase all from 0
    //context["post1"]->insert(0, "[-");
    //context["post1"]->append("-]");
    
    out << all << std::endl;
    byte_pool.clear();
    string_pool.clear();
}

int filter(int argc, char** argv)
{
    std::string line;
    rule rule1("(element) ([^ ]+) (.*)");
    
    while( std::getline(std::cin, line) ) {
        process_line(std::cout, line, rule1);
    }
    return 0;    
}

TINFRA_MAIN(filter);
