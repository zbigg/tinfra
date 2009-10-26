//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_tstring_h_included
#define tinfra_tstring_h_included

#include <string>
#include <stdexcept>
#include <vector>

#include <cassert>
#include <cstring>

namespace tinfra {

/**
    Temporary string that is valid in this scope and in all children scopes.
    
    After you've recived this string you rely on fact that it's caller
    responsibility to free this string. You mustn't modify it. You can pass it
    freely to new fun calls.
    
    This string is intended as read-only std::string const& replacement for parameters. It can be used
    in well defined scope: eg. sub-function call.
    Usually use of it doesn't require an initialization and it reuses C++ literals memory when 
    initialized with it.
 */

class string_pool;

class tstring  {
    const char* str_;
    size_t         length_;
    int         flags_;
public:
    tstring():
        str_(0),
        length_(0),
        flags_(0)
    {}

    template <int N>
    tstring(const char (&arr)[N]):
        str_((const char*)arr),
        length_(std::strlen(arr)),
        flags_(1)
    {
    }
    
    tstring(const char* str): 
        str_(str),
        length_(std::strlen(str)),
        flags_(1)
    {
    }
    
    
    tstring(const char* str, size_t length, bool has_null_terminate = false): 
        str_(str),
        length_(length),
        flags_(has_null_terminate ? 1 : 0)
    {
    }
    
    tstring(std::string const& str): 
        str_(str.c_str()),
        length_(str.size()),
        flags_(1)
    {
    }
    
    tstring(tstring const& other) : 
        str_(other.str_),
        length_(other.length_),
        flags_(other.flags_)
    { 
    }
    
    bool is_null_terminated() const { return flags_ == 1; }
    
    char const*  data() const  { return str_; }
    /*
    char const*  c_str() const
    { 
        if( is_null_terminated() )
	    return str_;
        throw std::runtime_error("c_str() called on non null-terminated string"); 
    }
    */
    char const*  c_str(string_pool& pool) const
    {
        if( is_null_terminated() ) 
            return str_;
        return temporary_alloc(pool, *this);
    }
    
    std::string  str()   const  { return std::string(tstring::data(), tstring::size()); }
    operator std::string() const { return str(); }
    
    //operator char const*() const { return data(); }
    
    char        operator[](size_t n) const { return tstring::data()[n]; }
    
    size_t size()       const { return length_; }

    typedef char   char_type;
    typedef size_t size_type;
    
    typedef char_type const*  iterator;
    typedef char_type const*  const_iterator;

    tstring substr(size_type pos) const
    {
        assert(pos <= size());
        return tstring(data() + pos, size()-pos, is_null_terminated());
    }
    tstring substr(size_type pos, size_type n) const;
    
    int      cmp(tstring const& other) const;
    
    bool operator == (const tstring& other) const { return cmp(other) == 0; }
    bool operator != (const tstring& other) const { return cmp(other) != 0; }
    
    bool operator >  (const tstring& other) const { return cmp(other) > 0; }    
    bool operator <  (const tstring& other) const { return cmp(other) < 0; }
    
    bool operator >= (const tstring& other) const { return cmp(other) >= 0; }    
    bool operator <= (const tstring& other) const { return cmp(other) <= 0; }
    
    iterator begin() { return data(); }
    iterator end() { return data()+tstring::size(); }
    
    const_iterator begin() const { return data(); }
    const_iterator end()   const { return data() + size(); }
    
    const_iterator rbegin() const { return data() + size() - 1; }
    const_iterator rend()   const { return data() - 1; }
    
    size_type find(tstring const& s, size_type pos = 0) const 
    {
        return this->find(s.data(), pos, s.size());
    }
    
    size_type find(char_type const* s, size_type pos, size_type n) const;
    // find first of
    size_type find_first_of(tstring const& s, size_type pos = 0) const
    {
        return this->find_first_of(s.data(), pos, s.size());
    }
    size_type find_first_of(char_type const* s, size_type pos, size_type n) const;
    size_type find_first_of(char_type c, size_type pos = 0) const;
    
    // find first not of
    size_type find_first_not_of(tstring const& s, size_type pos = 0) const
    {
        return this->find_first_not_of(s.data(), pos, s.size());
    }
    size_type find_first_not_of(char_type const* s, size_type pos, size_type n) const;
    size_type find_first_not_of(char_type c, size_type pos = 0) const;
    
    // find last of
    size_type find_last_of(tstring const& s, size_type pos = npos) const
    {
        return this->find_last_of(s.data(), pos, s.size());
    }
    size_type find_last_of(char_type const* s, size_type pos, size_type n) const;
    size_type find_last_of(char_type c, size_type pos = npos) const;
    
    // find last not of
    size_type find_last_not_of(tstring const& s, size_type pos = npos) const
    {
        return this->find_last_not_of(s.data(), pos, s.size());
    }
    
    size_type find_last_not_of(char_type const* s, size_type pos, size_type n) const;    
    size_type find_last_not_of(char_type c, size_type pos = npos) const;
    
    static const size_type npos;

private:
    static const char* temporary_alloc(string_pool& pool, tstring const& s);
};

std::ostream& operator<<(std::ostream& out, tstring const& s);

inline bool operator==(tstring const& a, const char* b)
{
    return a.cmp(tstring(b)) == 0;
}

inline bool operator==(const char* a, tstring const& b)
{
    return b.cmp(tstring(a)) == 0;
}

inline bool operator==(tstring const& a, std::string const& b)
{
    return a.cmp(tstring(b)) == 0;
}

inline bool operator==(std::string const& a, tstring const& b)
{
    return b.cmp(tstring(a)) == 0;
}

/// String pool
///
/// tstring class doesn't guarantee that it's string is ended with 0
/// Use this utility in contexts when you need null-terminated strings
/// like system calls.

class string_pool {
	std::vector<char*> strings;
public:
	const char* create(tstring const& s);
        tstring     alloc(tstring const& in) {
            const char* s = create(in);
            return tstring(s, in.size(), true);
        }

	string_pool(size_t initial_size = 128);
	~string_pool();
        
        void clear();
};

} // end of namespace tinfra


#endif

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

