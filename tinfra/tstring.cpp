#include <iostream>
#include <stdexcept>

#include "tinfra/thread.h"
#include "tinfra/cmd.h"

#include "tstring.h"

#include <pthread.h>

#if TINFRA_TSTING_CHECKS
namespace stack_traits {
    // TODO: revise this weird stack/heap detection heuristics
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

#endif
namespace tinfra {
    namespace tstring_detail {
        bool bad_abort = false;
        
        void tstring_set_bad_abort(bool a)
        {
            bad_abort = a;
        }
    }
    
#if TINFRA_TSTING_CHECKS
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
#endif

// find first of
tstring::size_type 
tstring::find_first_of(char_type const* s, size_type pos, size_type n) const
{
    if( tstring::size() > 0 ) {
        for( const_iterator i = begin()+pos; i != end(); ++i ) {
            if( std::memchr(s, *i, n) != 0 ) // 'not in S' so return
                return i - begin();
        }
    }
    return npos;
}
tstring::size_type 
tstring::find_first_of(char_type c, size_type pos) const
{
    if( tstring::size() > 0 ) {
        for( const_iterator i = begin()+pos; i != end(); ++i ) {
            if( *i == c ) // '== C' so return
                return i - begin();
        }
    }
    return npos;
}

// find first not of

tstring::size_type
tstring::find_first_not_of(char_type const* s, size_type pos, size_type n) const
{
    if( tstring::size() > 0 ) {
        for( const_iterator i = begin()+pos; i != end(); ++i ) {
            if( std::memchr(s, *i, n) == 0 ) // 'not in S' so return
                return i - begin();
        }
    }
}

tstring::size_type
tstring::find_first_not_of(char_type c, size_type pos) const
{
    if( tstring::size() > 0 ) {
        for( const_iterator i = begin()+pos; i != end(); ++i ) {
            if( *i != c )
                return i - begin();
        }
    }
    return npos;
}

// find last of

tstring::size_type
tstring::find_last_of(char_type const* s, size_type pos, size_type n) const
{
    if( tstring::size() > 0 ) {
        // TODO: pos not used!
        for( const_iterator i = rbegin(); i != rend(); --i ) {
            if( std::memchr(s, *i, n) != 0 ) // 'in S' so return
                return i - begin();
        }
    }
    return npos;
}

tstring::size_type
tstring::find_last_of(char_type c, size_type pos) const
{
    if( tstring::size() > 0 ) {
        // TODO: pos not used!
        for( const_iterator i = rbegin(); i != rend(); --i ) {
            if( *i == c ) // '== C' so return
                return i - begin();
        }
    }
    return npos;
}

// find last not of

tstring::size_type
tstring::find_last_not_of(char_type const* s, size_type pos, size_type n) const
{
    if( tstring::size() > 0 ) {
        // TODO: pos not used!
        for( const_iterator i = rbegin(); i != rend(); --i ) {
            if( std::memchr(s, *i, n) == 0 ) // 'not == S' so return
                return i - begin();
        }
    }
    return npos;
}

tstring::size_type
tstring::find_last_not_of(char_type c, size_type pos) const
{
    if( tstring::size() > 0 ) {
        // TODO: pos not used!
        for( const_iterator i = rbegin(); i != rend(); --i ) {
            if( *i != c ) // 'not = C' so return
                return i - begin();
        }
    }
    return npos;
}
    
} // end namespace tinfra

std::ostream& operator<<(std::ostream& out, tinfra::tstring const& s)
{
    return out.write(s.data(), s.size());
}

