//
// Copyright (c) 2012, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_mo_algo_h_included
#define tinfra_mo_algo_h_included

#include "mo.h"

namespace tinfra {

template <typename T>
bool mo_equals(T const& a, T const& b);

template <typename T>
bool mo_less_than(T const& a, T const& b);

template <typename T>
void mo_swap(T& a, T& b);

} // end namespace tinfra


//
// implementation detail
//
namespace tinfra {
namespace detail {    
    
template <typename T>
T& ref_in_other(const void* base_a, const T* value_in_a, void* base_b)
{
    const char* cp_base_a = reinterpret_cast<const char*>(base_a);
    const char* cp_value_in_a = reinterpret_cast<const char*>(value_in_a);
    char* cp_base_b = reinterpret_cast<char*>(base_b);
    
    std::ptrdiff_t offset = (cp_value_in_a - cp_base_a);
    
    char* cp_value_in_b = cp_base_b + offset;
    
    return reinterpret_cast<T&>( *cp_value_in_b );
}

template <typename T>
T const& const_ref_in_other(const void* base_a, const T* value_in_a, const void* base_b)
{
    const char* cp_base_a = reinterpret_cast<const char*>(base_a);
    const char* cp_value_in_a = reinterpret_cast<const char*>(value_in_a);
    const char* cp_base_b = reinterpret_cast<const char*>(base_b);
    
    std::ptrdiff_t offset = (cp_value_in_a - cp_base_a);
    
    const char* cp_value_in_b = cp_base_b + offset;
    
    return reinterpret_cast<T const&>( *cp_value_in_b );
}

template <typename Functor>
class relation_finder {
    Functor&    functor;
    const void* a;
    const void* b;
public:
    bool result_known;
    
    relation_finder(Functor& functor, const void* pa, const void* pb):
        functor(functor),
        a(pa), b(pb),
        result_known(false)
    {}

    template <typename S, typename T>
    void leaf(S const&, T const& va)
    {
        if( this->result_known )
            return;
        
        T const& vb = const_ref_in_other(this->a, &va, this->b);
        if( this->functor(va, vb) ) {
            this->result_known = true;
        }
            
    }
    
    template <typename S, typename T>
    void record(S const&, T const& va)
    {
        if( this->result_known )
            return;
        
        T const& vb = const_ref_in_other(this->a, &va, this->b);
        relation_finder<Functor> f(this->functor, &va, &vb);
        tinfra::mo_mutate(va, f);
        if( f.result_known ) {
            this->result_known = true;
        }
    }
};

class swapper {
    void* a;
    void* b;
public:
    swapper(void* pa, void* pb): a(pa), b(pb) {}

    template <typename S, typename T>
    void leaf(S const&, T& va)
    {
        T& vb = ref_in_other(this->a, &va, this->b);
        using std::swap;
        std::swap(va, vb);
    }
    
    template <typename S, typename T>
    void record(S const&, T& va)
    {
        T& vb = ref_in_other(this->a, &va, this->b);
        swapper f(&va, &vb);
        tinfra::mo_mutate(va, f); 
    }
};

} // end namespace tinfra::detail

struct equality_checker {
    template <typename T>
    bool operator()(T const& a, T const& b) {
        if( a == b )
            return false;
        else
            return true;
    }
};

template <typename T>
bool mo_equals(T const& a, T const& b)
{
    equality_checker relation_finder;
    tinfra::detail::relation_finder<equality_checker> f(relation_finder, &a, &b);
    
    // hmm, this "" is problematic
    tinfra::process("", a, f);
    
    return !f.result_known;
}

struct less_than_checker {
    bool result;
    
    template <typename T>
    bool operator()(T const& a, T const& b) {
        //std::cerr << "less_than_checker '" << a << "' >= '" << b << "' -> ";
        if( a < b ) { 
            this->result = true;
            //std::cerr << "a<b -> true, done\n";            
            return true;
        } else if( b < a ) {
            //std::cerr << "b<a -> false, done\n";
            this->result = false;
            return true;
        } else {
            //std::cerr << "b==a -> ...\n";
            return false;
        }
    }
};

template <typename T>
bool mo_less_than(T const& a, T const& b)
{
    less_than_checker relation_finder = { false };
    tinfra::detail::relation_finder<less_than_checker> f(relation_finder, &a, &b);
    
    // hmm, this "" is problematic
    tinfra::process("", a, f);
    
    return f.result_known && relation_finder.result;
}


template <typename T>
void mo_swap(T& a, T& b)
{
    tinfra::detail::swapper s(&a, &b);
    // hmm, this "" is problematic
    tinfra::mutate("", a, s);
}


} // end namespace tinfra

#endif // include guard

