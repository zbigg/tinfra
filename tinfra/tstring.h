#ifndef __tinfra_tstring_h__
#define __tinfra_tstring_h__

#include <string>

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
    
    There is no default constructor so you can't put into containers
    and it's hard to add as uninitialized field.
    
    Construction of instance from object that existed calls to sub functions consitutes
    an error which calls abort().
 */

// the weird stack pointer, stack detection and heap pointer detection
// is probably screwed up

#define TINFRA_TSTING_CHECKS 0
        
class tstring  {
    const char* str_;
    size_t      length_;
#if TINFRA_TSTING_CHECKS
    const void* stamp_;
#endif    
public:
    template <int N>
    tstring(const char (&arr)[N]):
        str_((const char*)arr),
        length_(std::strlen(arr))
#if TINFRA_TSTING_CHECKS
        ,stamp_(make_stamp(arr))
#endif        
    {
        //std::cerr << "created ARR tstring(" << this << ") stamp_: " << stamp_ << "\n";
        check_stamp();
    }
    
    tstring(const char* str): 
        str_(str),
        length_(std::strlen(str))
#if TINFRA_TSTING_CHECKS
        ,stamp_(make_stamp(this))
#endif
    {
        //std::cerr << "created PTR tstring(" << this << ") stamp_: " << stamp_ << "\n";
    }
    
    
    tstring(const char* str, size_t length): 
        str_(str),
        length_(length)
#if TINFRA_TSTING_CHECKS
        ,stamp_(make_stamp(this))
#endif        

    {
        //cerr << "created PTR tstring(" << this << ") stamp_: " << stamp_ << "\n";
    }
    
    tstring(std::string const& str): 
        str_(str.c_str()),
        length_(str.size())
#if TINFRA_TSTING_CHECKS
        ,stamp_(make_stamp(&str))
#endif        
    {
        //cerr << "created STD tstring(" << this << ") stamp_: " << stamp_ << "\n";
        check_stamp();
    }
    
    tstring(tstring const& other) : 
        str_(other.str_),
        length_(other.length_)
#if TINFRA_TSTING_CHECKS
        ,stamp_(other.stamp_)
#endif
    { 
        //cerr << "created TST tstring(" << this << ") stamp_: " << stamp_ << "\n";
        check_stamp(); 
    }
    
    char const*  data() const  { return str_; }
    
    char const*  c_str() const  { return str_; }
    
    std::string  str()   const  { return std::string(tstring::data(), tstring::size()); }
    
    //operator char const*() const { return data(); }
    
    char        operator[](size_t n) const { return tstring::data()[n]; }
    
    size_t size()       const { return length_; }

    typedef char   char_type;
    typedef size_t size_type;
    
    typedef char_type const*  iterator;
    typedef char_type const*  const_iterator;

    
    int cmp (tstring const& other) const {
        size_t common_length = std::min(size(), other.size());
        int r = std::memcmp(data(), other.data(), common_length);
        if( r != 0 ) 
            return r;
        return tstring::size() - other.size();
    }
    
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
    
    static const size_type npos = ~(size_type)0;
private:
#if TINFRA_TSTING_CHECKS
    static const void* make_stamp(const void* v)
    {
        return v;
    }
    void check_stamp();
#else
    void check_stamp() {}
    static const void* make_stamp(const void* v) { return 0; }
#endif
};

namespace tstring_detail {
    void tstring_set_bad_abort(bool a);
}

} // end of namespace tinfra

std::ostream& operator<<(std::ostream& out, tinfra::tstring const& s);

#endif

