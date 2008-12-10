#include <tinfra/cmd.h>
#include <cstdlib>

typedef void (*destroy_function_t)(void* f);

template<typename T>
    struct default_destructor {
        static void destroy(void* a)
        {
            T* _a = reinterpret_cast<T>(a);
            _a->~T();
        }
    };
    
template<typename T>
struct destructor_traits {
    static const intptr_t destroy_fn;
};

template <typename T>
const intptr_t destructor_traits<T>::destroy_fn = reinterpret_cast<intptr_t>(&default_destructor<T>::destroy);

template<> struct destructor_traits<char>          { static const intptr_t destroy_fn = 0; };
template<> struct destructor_traits<signed char>   { static const intptr_t destroy_fn = 0; };
template<> struct destructor_traits<unsigned char> { static const intptr_t destroy_fn = 0; };
template<> struct destructor_traits<int>           { static const intptr_t destroy_fn = 0; };
template<> struct destructor_traits<unsigned int>  { static const intptr_t destroy_fn = 0; };
template<> struct destructor_traits<long>          { static const intptr_t destroy_fn = 0; };
template<> struct destructor_traits<unsigned long> { static const intptr_t destroy_fn = 0; };

class marking_allocator {
public:
    template <typename T>
        void* alloc() {
            return alloc(sizeof(T), reinterpret_cast<destroy_function_t>(destructor_traits<T>::destroy_fn));
        }
    void* alloc(size_t size, destroy_function_t fn)
    {
        return ::malloc(size);
    }
};

int marking_allocator_test_main(int argc, char** argv)
{
    marking_allocator a;
    int* x = new(a.alloc<int>()) int(2);
    return 0;
}

TINFRA_MAIN(marking_allocator_test_main);
