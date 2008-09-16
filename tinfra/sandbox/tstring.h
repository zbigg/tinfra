#ifndef __tinfra_tstring_h__
#define __tinfra_tstring_h__

namespace tinfra {
    
/**
    Temporary string that is valid in this scope and in all children scopes.
    
    After you've recived this string you rely on fact that it's caller
    responsibility to free this string. You mustn't modify it. You can pass it
    freely to new fun calls.
    
    This string is intended as read-only std::string replacement which is used in defined scope.
    Usually use of it doesn't require an initialization and it reuses C++ literals memory when 
    when initialized with it.
    
    There is no default constructor so you can't put into containers
    and it's hard to add as uninitialized field    
 */
    
template <typename IMPL>
class string_traits {
    std::string  str()   const  { return std::string(c_str(), size()); }
    
    char operator[](size_t n) const { return str_[n]; }
};

class tstring {
    const char* str_;
    size_t      length_;
public:
    
    tstring(const char* str): str_(str), length_(std::strlen(str)) {}
    
    tstring(const char* str, int length): str_(str), length_(length) {}
    
    tstring(std::string const& str): str_(str.c_str()), length_(str.size()) {}
    
    tstring(tstring const& other): str_(other.str_), length_(other.length_) {}
        
    char const*  c_str() const  { return str_; }    
    
    
    operator char const*() const { return c_str(); }
    operator std::string() const { return str(); }
    
    size_t size()   const { return length_; }    
    
    
    
    int cmp (const istring& other) const {
        size_t common_length = std::min(length_, other.length_);
        int r = std::memcmp(str_, other.str_, common_length);
        if( r ) 
            return r;
        return length_ - other.length_;
    }
    
    bool operator == (const istring& other) const { return cmp(other) == 0; }
    bool operator != (const istring& other) const { return cmp(other) != 0; }
    
    bool operator >  (const istring& other) const { return cmp(other) > 0; }    
    bool operator <  (const istring& other) const { return cmp(other) < 0; }
    
    bool operator >= (const istring& other) const { return cmp(other) >= 0; }    
    bool operator <= (const istring& other) const { return cmp(other) <= 0; }
};

} // end namespace tinfra

#endif