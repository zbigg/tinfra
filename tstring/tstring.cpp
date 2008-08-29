#include <iostream>
#include <stdexcept>

#include "tinfra/tinfra.h"

#include "tinfra/cmd.h"

#include <pthread.h>

namespace stack_direction {
    void current_stack_region(void*& bottom, size_t& size)
    {
      pthread_attr_t attr;
      int res = pthread_getattr_np(pthread_self(), &attr);
      if (res != 0) {
          throw std::runtime_error("pthread_getattr_np");
      }
    
      char* stack_bottom;
      size_t stack_bytes;
      res = pthread_attr_getstack(&attr, (void **) &stack_bottom, &stack_bytes);
      if (res != 0) {
          pthread_attr_destroy(&attr);
          throw std::runtime_error("pthread_getattr_np");
      }
      char* stack_top = stack_bottom + stack_bytes;
#if 0
      // The block of memory returned by pthread_attr_getstack() includes
      // guard pages where present.  We need to trim these off.
      size_t page_bytes = os::Linux::page_size();
      assert(((intptr_t) stack_bottom & (page_bytes - 1)) == 0, "unaligned stack");
    
      size_t guard_bytes;
      res = pthread_attr_getguardsize(&attr, &guard_bytes);
      if (res != 0) {
        fatal1("pthread_attr_getguardsize failed with errno = %d", res);
      }
      int guard_pages = align_size_up(guard_bytes, page_bytes) / page_bytes;
      assert(guard_bytes == guard_pages * page_bytes, "unaligned guard");
    
    #ifdef IA64
      // IA64 has two stacks sharing the same area of memory, a normal
      // stack growing downwards and a register stack growing upwards.
      // Guard pages, if present, are in the centre.  This code splits
      // the stack in two even without guard pages, though in theory
      // there's nothing to stop us allocating more to the normal stack
      // or more to the register stack if one or the other were found
      // to grow faster.
      int total_pages = align_size_down(stack_bytes, page_bytes) / page_bytes;
      stack_bottom += (total_pages - guard_pages) / 2 * page_bytes;
    #endif // IA64
      
      stack_bottom += guard_bytes;
#endif
      pthread_attr_destroy(&attr);
      bottom = stack_bottom;
      size = stack_bytes;
      std::cerr << "stack_bottom=" << bottom << " size=" << size << "\n";
    }

    char* deeper_address()
    {
        char a;
        return &a;
    }
    int get_stamp_direction()
    {
        char a;
        char b;
        char* x = &a;
        char* y = deeper_address();
        char* z = &b;
        int r = y-x;
        std::cerr << "get_stamp_direction() -> " << r << "\n";
        return y-x;
    }
    bool is_on_heap(const void* v)
    {
        void* stack_bottom;
        size_t stack_size;
        current_stack_region(stack_bottom, stack_size);
        
        return v == 0 || v < stack_bottom;
        
    }
    bool is_valid_(const void* current, const void* stamp)
    {
        if( get_stamp_direction() < 0 )
            return stamp   >= current || is_on_heap(stamp);
        else
            return current <= stamp || is_on_heap(stamp);
    }
    
    bool is_valid(const void* current, const void* stamp)
    {
        bool r = is_valid_(current, stamp);
        std::cerr << "is_valid(" << current << ", " << stamp << ") -> " << r << "\n";
        return r;
    }
}

using std::cerr;
using std::endl;

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
        cerr << "created ARR tstring(" << this << ") stamp_: " << stamp_ << "\n";
        check_stamp();
    }
    
    tstring(const char* str): 
    	str_(str),
	length_(std::strlen(str)),
	stamp_(make_stamp(this))
    {
        cerr << "created PTR tstring(" << this << ") stamp_: " << stamp_ << "\n";
    }
    
    
    tstring(const char* str, int length): 
    	str_(str),
	length_(length),
	stamp_(make_stamp(this))
    {
        cerr << "created PTR tstring(" << this << ") stamp_: " << stamp_ << "\n";
    }
    
    tstring(std::string const& str): 
    	str_(str.c_str()),
	length_(str.size()),
	stamp_(make_stamp(&str))
    {
        cerr << "created STD tstring(" << this << ") stamp_: " << stamp_ << "\n";
        check_stamp();
    }
    
    tstring(tstring const& other) : 
    	str_(other.str_),
	length_(other.length_),
	stamp_(other.stamp_)
    { 
        cerr << "created TST tstring(" << this << ") stamp_: " << stamp_ << "\n";
        check_stamp(); 
    }
        
    char const*  c_str() const  { return str_; }    
    
    operator char const*() const { return c_str(); }
    
    size_t size()   const { return length_; }    
private:
    static const void* make_stamp(const void* v)
    {
	 return v;
    }
    void check_stamp()
    {
        if( !stack_direction::is_valid(this, stamp_) ) {
            //abort();
            throw std::logic_error("invalid tstring usage!, returned from function");
        }
    }
};

template <int N>
void foo(char (&a)[N])
{
    std::cerr << "fooARR" << std::endl; 
}

void foo(const char*)
{
    std::cerr << "fooSTR" << std::endl;
}
tstring badsanta()
{
    char buf[1024] = "anc";
    std::cerr << tinfra::demangle_typeinfo_name(typeid(buf)) << std::endl;
    foo(buf);
    tstring a(buf);
    return a;
}
std::string dupaddd = "kkkk";

int stackstring_main(int argc, char** argv)
{
    tstring a = "zbyszek";
    tstring b = a;
    tstring c(b);
    std::string* ds = new std::string("yo");
    tstring d(*ds);
    tstring e(dupaddd);
    //tstring e = badsanta();
    
    //std::cout << e.c_str() << std::endl;
}

TINFRA_MAIN(stackstring_main);

