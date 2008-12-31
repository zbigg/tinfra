//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "tinfra/thread.h"
#include "tinfra/cmd.h"
#include <cstdlib>

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

const tstring::size_type tstring::npos;

tstring tstring::substr(size_type pos, size_type n) const
{
    assert(pos <= size());
    const size_t len = std::min(size()-pos, n);
    const size_t last_character_pos = pos+len;
    bool sub_is_null_terminated = 
           ( last_character_pos == size() && this->is_null_terminated())
        || ( last_character_pos <  size() && data()[last_character_pos] == '\0' );
    return tstring(data() + pos, len, sub_is_null_terminated);
}

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
    return npos;
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
    

std::ostream& operator<<(std::ostream& out, tstring const& s)
{
    return out.write(s.data(), s.size());
}

const char* tstring::temporary_alloc(string_pool& pool, tstring const& s)
{
    return pool.create(s);
}

const char* string_pool::create(tstring const& src)
{
    strings.push_back(0);
    size_t len = src.size();
    char* result = reinterpret_cast<char*>( std::malloc(len+1) );
    strings[strings.size()-1] = result;
    
    std::memcpy(result, src.data(), len);
    result[len] = 0;
    return result;
}

void string_pool::clear()
{
    for( size_t i = 0; i < strings.size(); ++i ) {
        std::free(strings[i]);
        strings[i] = 0;
    }
    strings.clear();
}
string_pool::string_pool(size_t initial_size)
{
}

string_pool::~string_pool()
{
    clear();
}

} // end namespace tinfra

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:


