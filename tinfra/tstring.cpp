#include <iostream>
#include <stdexcept>

#include "tinfra/thread.h"
#include "tinfra/cmd.h"

#include "tstring.h"

#include <pthread.h>

namespace stack_traits {
    void current_stack_region(void*& bottom, size_t& size)
    {
#if defined(linux)
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
        
        pthread_attr_destroy(&attr);
        bottom = stack_bottom;
        size = stack_bytes;
#else
        char a;
        size = 1000000;
        bottom = (&a - size);
#endif
        std::cerr << "thread=" << pthread_self() << " stack_bottom=" << bottom << " size=" << size << "\n";
    }

    /*
    char* deeper_address()
    {
        char a;
        return &a;
    }
    */
    int get_stack_direction()
    {
        char a;
        char b;
        char* x = &a;
        char* y = &b; // deeper_address();
        int r = y-x;
        //std::cerr << "get_stack_direction() -> " << r << "\n";
        return y-x;
    }
    
    __thread void* stack_bottom = 0;
    
    bool is_on_heap(const void* v)
    {
        //void* stack_bottom = 0;
        if( stack_bottom == 0 ) {
            size_t stack_size;
            current_stack_region(stack_bottom, stack_size);
        }
        bool r = (v == 0) || (v < stack_bottom);
        std::cerr << "is_on_heap(" << v << ") -> " << r << "\n";
        return r;
    }
    bool is_valid_(const void* current, const void* stamp)
    {
        static int stack_direction = 0;
        if( stack_direction == 0 )
            stack_direction = get_stack_direction();
        bool r;
        if( stack_direction < 0 )
            r = stamp   >= current;
        else
            r = current <= stamp;
        r = r || is_on_heap(stamp);
        return r;
    }
    
    bool is_valid(const void* current, const void* stamp)
    {
        bool r = is_valid_(current, stamp);
        //std::cerr << "is_valid(" << current << ", " << stamp << ") -> " << r << "\n";
        return r;
    }
} // end namespace stack_traits

namespace tinfra {
    namespace tstring_detail {
        bool bad_abort = false;
        
        void tstring_set_bad_abort(bool a)
        {
            bad_abort = a;
        }
    }
    void tstring::check_stamp()
    {
        if( !stack_traits::is_valid(this, stamp_) ) {
            if( tstring_detail::bad_abort ) {
                abort();
            } else {
                throw std::logic_error("invalid tstring usage!");
            }
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

int test_param(tstring const& a)
{
    return a.size() > 0; 
}
void test_tstring()
{
    std::string ss = "hello";
    char buf[1024] = "hhh";
    
    tstring a = "zbyszek";
    tstring b = a;
    tstring c(b);
    std::string* ds = new std::string("yo");
    tstring d(*ds);
    tstring e(dupaddd);
    delete ds;
    
    test_param(ss);
    test_param("aa");
    test_param(buf);
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
    //ts.start(&test_in_other_thread,0);
    //ts.join();
    
    ts.join();
    return 0;
}

TINFRA_MAIN(stackstring_main);

