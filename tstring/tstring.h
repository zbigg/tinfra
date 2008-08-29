#ifndef __tinfra_tstring_h__
#define __tinfra_tstring_h__


namespace tinfra {
template <typename IMPL>
class string_traits {
    std::string  str()   const  { return std::string(IMPL::c_str(), IMPL::size()); }
    
    operator char const*() const { return IMPL::c_str(); }
    
    char        operator[](size_t n) const { return IMPL::c_str()[n]; }
    
    int cmp (IMPL const& other) const {
	size_t common_length = std::min(IMPL::size(), other.size());
        int r = std::memcmp(IMPL::c_str(), other.c_str(), common_length);
        if( r ) 
            return r;
        return IMPL::size() - other.size();
    }
    
    bool operator == (const IMPL& other) const { return cmp(other) == 0; }
    bool operator != (const IMPL& other) const { return cmp(other) != 0; }
    
    bool operator >  (const IMPL& other) const { return cmp(other) > 0; }    
    bool operator <  (const IMPL& other) const { return cmp(other) < 0; }
    
    bool operator >= (const IMPL& other) const { return cmp(other) >= 0; }    
    bool operator <= (const IMPL& other) const { return cmp(other) <= 0; }
};

/**
    Temporary string that is valid in this scope and in all children scopes.
    
    After you've recived this string you rely on fact that it's caller
    responsibility to free this string. You mustn't modify it. You can pass it
    freely to new fun calls.
    
    This string is intended as read-only std::string replacement which is used in defined scope.
    Usually use of it doesn't require an initialization and it reuses C++ literals memory when 
    when initialized with it.
    
    There is no default constructor so you can't put into containers
    and it's hard to add as uninitialized field.
    
    Construction of instance from object that existed calls to sub functions consitutes
    an error which calls abort().
 */

class tstring: public string_traits<tstring> {
    const char* str_;
    size_t      length_;
    const void* stamp_;
public:
    template <int N>
    tstring(const char (&arr)[N]):
    	str_(arr),
	length_(std::strlen(arr)),
	stamp_(make_stamp(arr))
    {
        //cerr << "created ARR tstring(" << this << ") stamp_: " << stamp_ << "\n";
        check_stamp();
    }
    
    tstring(const char* str): 
    	str_(str),
	length_(std::strlen(str)),
	stamp_(make_stamp(this))
    {
        //cerr << "created PTR tstring(" << this << ") stamp_: " << stamp_ << "\n";
    }
    
    
    tstring(const char* str, int length): 
    	str_(str),
	length_(length),
	stamp_(make_stamp(this))
    {
        //cerr << "created PTR tstring(" << this << ") stamp_: " << stamp_ << "\n";
    }
    
    tstring(std::string const& str): 
    	str_(str.c_str()),
	length_(str.size()),
	stamp_(make_stamp(&str))
    {
        //cerr << "created STD tstring(" << this << ") stamp_: " << stamp_ << "\n";
        check_stamp();
    }
    
    tstring(tstring const& other) : 
    	str_(other.str_),
	length_(other.length_),
	stamp_(other.stamp_)
    { 
        //cerr << "created TST tstring(" << this << ") stamp_: " << stamp_ << "\n";
        check_stamp(); 
    }
        
    char const*  c_str() const  { return str_; }    
    
    operator char const*() const { return c_str(); }
    
    size_t size()       const { return length_; }
private:
    static const void* make_stamp(const void* v)
    {
	 return v;
    }
    void check_stamp();
};

} // end of namespace tinfra
#endif

