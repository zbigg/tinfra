#include <iostream>
#include <stdexcept>

#include "tinfra/thread.h"
#include "tinfra/cmd.h"

#include "tstring.h"

#include <pthread.h>

namespace stack_traits {
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
      //char* stack_top = stack_bottom + stack_bytes;
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
      std::cerr << "thread=" << pthread_self() << " stack_bottom=" << bottom << " size=" << size << "\n";
    }

    /*
    char* deeper_address()
    {
        char a;
        return &a;
    }
    */
    __thread void* stack_bottom = 0;
    int get_stamp_direction()
    {
        char a;
        char b;
        char* x = &a;
        char* y = &b; // deeper_address();
        int r = y-x;
        std::cerr << "get_stamp_direction() -> " << r << "\n";
        return y-x;
    }
    bool is_on_heap(const void* v)
    {
        //void* stack_bottom;
        if( stack_bottom == 0 ) {
            size_t stack_size;
            current_stack_region(stack_bottom, stack_size);
        }
        
        return v == 0 || v < stack_bottom;
        
    }
    bool is_valid_(const void* current, const void* stamp)
    {
        static int stack_direction = 0;
        if( stack_direction == 0 )
            stack_direction = get_stamp_direction(); 
        if( stack_direction < 0 )
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
} // end namespace stack_traits

namespace tinfra {
    void tstring::check_stamp()
    {
        if( !stack_traits::is_valid(this, stamp_) ) {
            abort();
            //throw std::logic_error("invalid tstring usage!, returned from function");
        }
    }
} // end namespace tinfra

using tinfra::tstring;

tstring badsanta()
{
    char buf[1024] = "anc";
    tstring a(buf);
    return a;
}
std::string dupaddd = "kkkk";

void test_tstring()
{
    tstring a = "zbyszek";
    tstring b = a;
    tstring c(b);
    std::string* ds = new std::string("yo");
    tstring d(*ds);
    tstring e(dupaddd);
}
void* test_in_other_thread(void* )
{
    test_tstring();
    return 0;
}
int stackstring_main(int argc, char** argv)
{
    test_tstring();
    tinfra::ThreadSet ts;
    ts.start(&test_in_other_thread,0);
    ts.join();
    return 0;
}

TINFRA_MAIN(stackstring_main);

