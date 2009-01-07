#ifndef tinfra_memory_pool_h
#define tinfra_memory_pool_h

#include <list>
#include <cstdlib>
#include <stdexcept>
namespace tinfra {

template <typename T>
class static_pool {    
    T* buffer;
    T* next;
    T* end;
    size_t size_;
public:    
    static_pool(size_t size)
        : buffer(0), next(0), end(0),size_(size) {}
    
    ~static_pool() { 
        clear(); 
        std::free(buffer);
    }
    
    T* alloc(size_t n = 1) {
        if( ! buffer ) {
            buffer = reinterpret_cast<T*>(std::malloc(sizeof(T)*size_));
            next = buffer;
            end = buffer + size_;            
        }
        T* const result = next;
        T* const new_next = next + n;
        
        if( new_next <= end ) {
            next = new_next;
            return result;
        } else {
            return 0;
        }
    }
    
    bool full() const { return next == end; }
    
    void clear() {
        for( T* i = buffer; i != next;  ++i ) {
            i->~T();
        }
        next = buffer;
    }
    
    size_t size() const {
        return buffer ? size_*sizeof(T) : 0;
    }
};

typedef static_pool<char> static_byte_pool;

#include <list>

template <typename T>
class pool {
public:
    pool(size_t chunk_size)
        : current_pool(0), 
          chunk_size(chunk_size) 
    {}
        
    ~pool() {
    }
    
    T*    construct() {
        T* r = alloc();
        if( r ) {
            new(r) T();
            return r;
        } else {
            return 0;
        }
    }
    
    T* alloc(size_t n=1) {
        while(true) {
            if( n > chunk_size )
                throw std::logic_error("attempt to allocate more than pool supports");

            if( current_pool ) {
                T* result = current_pool->alloc(n);
                if( result != 0 ) 
                    return result;
            }
            if ( !new_pool() ) 
                return 0;
        }
    }
    
    void clear()
    {
        for( typename pools_list::iterator i = subpools.begin(); i != subpools.end(); ++i )
            i->clear();
    }
    
    size_t size() const {
        size_t result;
        for( typename pools_list::const_iterator i = subpools.begin(); i != subpools.end(); ++i )
            result += i->size();
        return result;
    }
private:
    typedef static_pool<T> subpool_t;
    typedef std::list<subpool_t> pools_list;

    pools_list subpools;
    subpool_t* current_pool;
    size_t chunk_size;

    bool new_pool() {
        /*
        for( typename pools_list::iterator i = subpools.begin(); i != subpools.end(); ++i ) {
            if( ! i->full() ) {
                current_pool = & (*i);
                return true;
            }
        }
        */
        subpools.push_back(subpool_t(chunk_size));
        current_pool = &subpools.back();
        return true;
    }
};

typedef pool<char> raw_memory_pool;

} // end namespace tinfra

#endif // tinfra_memory_pool_h
