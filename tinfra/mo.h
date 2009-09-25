//
// Copyright 2009 (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_mo_h_included__
#define tinfra_mo_h_included__

#include "tinfra/symbol.h"

namespace tinfra {
/** MO structure trait

mo_traits<T> is template class that provides means for MO algorithms to
know if process value with functor.

Default mo_traits<T> applies functor to value thus trating value as leaf.
    
For structures that has children - objects or containers one should specialize
mo_traits<V> and provide specialized walk mechanism:
 * struct_mo_trait is for MO structures
 * conatiner_mo_trait is for STL compatible containers (only processing 
   is provided)
    
*/
template <typename T>
struct mo_traits {
    template <typename F>
    static void process(const symbol& s, const T& v, F& f) { 
        f(s,v);
    }
    
    template <typename F>
    static void mutate(const symbol& s, T& v, F& f) { 
        f(s,v);
    }
};

template <typename T>
struct struct_mo_traits {
    template <typename Functor>
    static void process(const symbol& s, const T& v, Functor& f) { 
        f.mstruct(s, v);
    }
    
    template <typename Functor>
    static void mutate(const symbol& s, T& v, Functor& f) { 
        f.mstruct(s, v);
    }
};

template <typename T>
struct container_mo_traits{
    template <typename Functor>
    static void process(const symbol& s, const T& v, Functor& f) {
        f.container(s, v);
    }
    
    
    template <typename Functor>
    static void mutate(const symbol& s, T& v, Functor& f) { 
        f.container(s, v);
    }
};

template <typename T>
struct mo_traits < std::vector<T> >: 
    public container_mo_traits< std::vector<T> > { };

template <typename T, typename F>
void mo_mutate(T& value, F& functor);

namespace mo {

template <typename Functor>
struct dispatcher{
    Functor& f;
    dispatcher(Functor& _f) : f(_f) {}
            
    template <typename V>
    void operator() (const symbol& s, const V& v)  { 
        mo_traits<V>::process(s, v, f); 
    }
    
    template <typename V>
    void operator() (const symbol& s, V& v)  { 
        mo_traits<V>::mutate(s, v, f); 
    }
};

template <typename Functor>
struct mutate_helper {
    Functor& mutator_;
    mutate_helper(Functor& mutator) : mutator_(mutator) {}
    
    template <class V>
    void operator () (const symbol& sym, const V& v) {
        mutator_(sym, const_cast<V&>(v));
    }
    
    template <typename T>
    void mstruct(symbol const&, T& v) {
        mo_mutate(v, mutator_);
    }
};

} // end namespace mo_detail

template <typename T, typename F>
void mo_process(T const& value, F& functor)
{
    mo::dispatcher<F> functor_disp(functor);
    value.apply(functor_disp);
}

template <typename T, typename F>
void mo_mutate(T& value, F& functor)
{
    mo::mutate_helper<F> mutator(functor);
    mo::dispatcher< mo::mutate_helper<F> > functor_disp(mutator);
    value.apply(functor_disp);
}

template <typename T, typename F>
void process(symbol const& sym,  T const& value, F& functor)
{
    mo::dispatcher<F> functor_disp(functor);
    functor_disp(sym, value);
}

template <typename T, typename F>
void mutate(symbol const& sym,  T& value, F& functor)
{
    mo::dispatcher<F> functor_disp(functor);
    functor_disp(sym, value);
}


#define TINFRA_MO_MANIFEST(a)  template <typename F> void apply(F& f) const

#define TINFRA_MO_FIELD(a)    f(S::a, a)
    
#define TINFRA_SYMBOL_DECL(a) namespace S { extern tinfra::symbol a; } extern int TINFRA_SYMBOL_DECL_ ## a
#define TINFRA_SYMBOL_IMPL(a) namespace S { tinfra::symbol a(#a); } extern int TINFRA_SYMBOL_IMPL_ ## a

} // end namespace tinfra

#endif // tinfra_mo_h_included__

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
