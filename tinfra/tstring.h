//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_tstring_h_included
#define tinfra_tstring_h_included

#include "platform.h" // fot TINFRA_CONSTEXPR && TINFRA_NOEXCEPT

#include <string>  // for std::string
#include <vector>  // for std::vector used in string_pool

#include <cassert> // for assert
#include <cstring> // for std::strlen

#include <limits>  // for std::numeric_limits


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

    Note:
        It is considered subset of std::string_view (http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3609.html)
        as proposed to C++1y. Subset because current `tstring` is ITSELF IMMUTABLE and points at IMMUTABLE string
        as opposed to std::string_view which is MUTABLE (still pointing to IMMUTABLE buffer).
    
        Now i consider upgrading tstring to fully compatible replacement of string_view i.e making it MUTABLE.
 */

class string_pool;

class tstring  {
    const char* str_;
    size_t         length_;
    int         flags_;
public:
    // types
    typedef char   char_type;
    typedef char_type value_type;

    typedef size_t size_type;
    typedef std::ptrdiff_t difference_type;

    typedef char_type const*  iterator;
    typedef char_type const*  const_iterator;

    typedef char_type const&  reference;
    typedef char_type const&  const_reference;

    // [string.view.cons], construct/copy
    TINFRA_CONSTEXPR tstring() TINFRA_NOEXCEPT;
    template <int N>
        TINFRA_CONSTEXPR tstring(const char (&arr)[N])  TINFRA_NOEXCEPT;
    /*TINFRA_CONSTEXPR*/ tstring(const char* str);
        // note, this should be constexpr, but clang still can't do it with strlen()
    TINFRA_CONSTEXPR tstring(const char* str, size_t length, bool has_null_terminate = false);
    /*TINFRA_CONSTEXPR*/ tstring(std::string const& str) TINFRA_NOEXCEPT;
    TINFRA_CONSTEXPR tstring(tstring const& other) TINFRA_NOEXCEPT /* = default */;

    //TINFRA_CONSTEXPR tstring& operator=(tstring const& other) TINFRA_NOEXCEPT /* = default */;

    // custom tstring interface
    bool is_null_terminated() const { return flags_ == 1; }
    char const*  c_str(string_pool& pool) const
    {
        if( is_null_terminated() ) 
            return str_;
        return temporary_alloc(pool, *this);
    }

    std::string  str()     const  { return std::string(tstring::data(), tstring::size()); }
    operator std::string() const { return str(); }

    // [string.view.iterators], iterators
    TINFRA_CONSTEXPR iterator  begin()       const TINFRA_NOEXCEPT { return data(); }
    TINFRA_CONSTEXPR iterator end()          const TINFRA_NOEXCEPT { return data() + size(); }
    TINFRA_CONSTEXPR const_iterator cbegin() const TINFRA_NOEXCEPT { return data(); }
    TINFRA_CONSTEXPR const_iterator cend()   const TINFRA_NOEXCEPT { return data() + size(); }
    
    // TBD, not sure what is exact meaning of reverse_iterator
    iterator       rbegin() const TINFRA_NOEXCEPT { return data() + size() - 1; }
    iterator       rend()   const TINFRA_NOEXCEPT { return data() - 1; }

    // [string.view.capacity], capacity
    TINFRA_CONSTEXPR size_t size()     const TINFRA_NOEXCEPT { return length_; }
    TINFRA_CONSTEXPR bool   empty()    const TINFRA_NOEXCEPT { return length_ == 0; }
    TINFRA_CONSTEXPR size_t length()   const TINFRA_NOEXCEPT { return length_; };
    TINFRA_CONSTEXPR size_t max_size() const TINFRA_NOEXCEPT { return std::numeric_limits<size_t>::max(); }

    // [string.view.access], element access
    TINFRA_CONSTEXPR const char&        operator[](size_t n) const { return data()[n]; }
    TINFRA_CONSTEXPR const char&        at(size_t n) const { return data()[n]; }
    TINFRA_CONSTEXPR const char&  front() const { return *str_; };
    TINFRA_CONSTEXPR const char&  back() const  { return *( str_ + length_ - 1); };
    TINFRA_CONSTEXPR char const*  data() const TINFRA_NOEXCEPT { return str_; }

    // [string.view.modifiers], modifiers:
    //void clear() TINFRA_NOEXCEPT;
    //void remove_prefix(size_type n);
    //void remove_suffix(size_type n);

    // [string.view.ops], string operations:

    tstring substr(size_type pos) const;
    tstring substr(size_type pos, size_type n) const;
    
    int      compare(tstring const& other) const;
    int      cmp(tstring const& other) const; // deprecated
    
    TINFRA_CONSTEXPR bool starts_with(tstring s) const TINFRA_NOEXCEPT;
    TINFRA_CONSTEXPR bool starts_with(char_type c) const TINFRA_NOEXCEPT;
    TINFRA_CONSTEXPR bool starts_with(const char_type* s) const TINFRA_NOEXCEPT;
    TINFRA_CONSTEXPR bool ends_with(tstring s) const TINFRA_NOEXCEPT;
    TINFRA_CONSTEXPR bool ends_with(char_type c) const TINFRA_NOEXCEPT;
    TINFRA_CONSTEXPR bool ends_with(const char_type* s) const TINFRA_NOEXCEPT;
    
    bool operator == (const tstring& other) const { return cmp(other) == 0; }
    bool operator != (const tstring& other) const { return cmp(other) != 0; }
    
    bool operator >  (const tstring& other) const { return cmp(other) > 0; }
    bool operator <  (const tstring& other) const { return cmp(other) < 0; }
    
    bool operator >= (const tstring& other) const { return cmp(other) >= 0; }
    bool operator <= (const tstring& other) const { return cmp(other) <= 0; }
    
    
    
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

//
// tstring (inline) implementation
//

TINFRA_CONSTEXPR
inline
tstring::tstring() TINFRA_NOEXCEPT:
    str_(0),
    length_(0),
    flags_(0)
{}

template <int N>
TINFRA_CONSTEXPR
tstring::tstring(const char (&arr)[N]) TINFRA_NOEXCEPT:
    str_((const char*)arr),
    length_(std::strlen(arr)),
    flags_(1)
{
}

inline
//TINFRA_CONSTEXPR
tstring::tstring(const char* str) :
    str_(str),
    length_(std::strlen(str)),
    flags_(1)
{
}

inline
TINFRA_CONSTEXPR
tstring::tstring(const char* str, size_t length, bool has_null_terminate):
    str_(str),
    length_(length),
    flags_(has_null_terminate ? 1 : 0)
{
}

inline
//TINFRA_CONSTEXPR
tstring::tstring(std::string const& str) TINFRA_NOEXCEPT:
    str_(str.data()),
    length_(str.size()),
    flags_(1)
{
}

inline
TINFRA_CONSTEXPR
tstring::tstring(tstring const& other) TINFRA_NOEXCEPT:
    str_(other.str_),
    length_(other.length_),
    flags_(other.flags_)
{
}

inline
TINFRA_CONSTEXPR bool tstring::starts_with(tstring s) const TINFRA_NOEXCEPT
{
    return (this->size() < s.size() )
            ? false
            : std::memcmp(data(), s.data(), s.size()) == 0;
}

inline
TINFRA_CONSTEXPR bool tstring::starts_with(char_type c) const TINFRA_NOEXCEPT
{
    return ( this->size() == 0 ) 
            ? false
            : this->front() == c;
}

inline
TINFRA_CONSTEXPR bool tstring::starts_with(const char_type* s) const TINFRA_NOEXCEPT
{
#ifdef TINFRA_CXX11
    // in C++ we're constexpr so we need to
    // be single statement
    return    (*s == 0 )            ? true
            : (this->size() == 0 )  ? false
            : (this->front() == *s) ? this->substr(1).starts_with(s+1)
                                    : false;
#else
    // pre C++11, can be iterative
          iterator i = this->begin();
    const iterator e = this->end();
    while( i < e && *s ) {
        if( *s != *i ) {
            return false;
        }
        i++;
        s++;
    }
    return true;
#endif
}

inline
TINFRA_CONSTEXPR bool tstring::ends_with(tstring s) const TINFRA_NOEXCEPT
{
    return ( this->size() < s.size() )
        ? false
        : ( std::memcmp(data() + this->size() - s.size(), s.data(), s.size()) == 0 );
}

inline
TINFRA_CONSTEXPR bool tstring::ends_with(char_type c) const TINFRA_NOEXCEPT
{
    return ( this->size() == 0 ) 
        ? false
        : this->back() == c;
}

inline
TINFRA_CONSTEXPR bool tstring::ends_with(const char_type* s) const TINFRA_NOEXCEPT
{
    return this->ends_with(tstring(s,strlen(s)));
}

inline
tstring tstring::substr(tstring::size_type pos) const
{
    assert(pos <= size());
    return tstring(data() + pos, size()-pos, is_null_terminated());
}

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

