//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_mo_h_included__
#define tinfra_mo_h_included__

#include <vector> // TBD, remove for vector

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
    template <typename S, typename F>
    static void process(S const& s, const T& v, F& f) { 
        f.leaf(s,v);
    }
    
    template <typename S, typename F>
    static void mutate(S const& s, T& v, F& f) { 
        f.leaf(s,v);
    }
};

template <typename T>
struct struct_mo_traits {
    template <typename S, typename Functor>
    static void process(S const& s, const T& v, Functor& f) { 
        f.record(s, v);
    }
    
    template <typename S, typename Functor>
    static void mutate(S const& s, T& v, Functor& f) { 
        f.record(s, v);
    }
};

template <typename T>
struct container_mo_traits{
    template <typename S, typename Functor>
    static void process(S const& s, const T& v, Functor& f) {
        f.sequence(s, v);
    }
    
    
    template <typename S, typename Functor>
    static void mutate(S const& s, T& v, Functor& f) { 
        f.sequence(s, v);
    }
};

template <typename T>
struct mo_traits < std::vector<T> >: 
    public container_mo_traits< std::vector<T> > { };

template <typename T, typename F>
void mo_mutate(T& value, F& functor);

namespace mo {

template <typename Functor>
struct dispatcher {
    Functor& f;
    dispatcher(Functor& _f) : f(_f) {}
            
    template <typename S, typename V>
    void dispatch(S const& s, const V& v)  { 
        mo_traits<V>::process(s, v, f); 
    }
    
    template <typename S, typename V>
    void dispatch(S const& s, V& v)  { 
        mo_traits<V>::mutate(s, v, f); 
    }
};

template <typename Functor>
struct mutate_helper {
    Functor& mutator_;
    mutate_helper(Functor& mutator) : mutator_(mutator) {}
    
    template <typename S, class V>
    void leaf(S const& sym, const V& v) {
        mutator_.leaf(sym, const_cast<V&>(v));
    }
    
    template <typename S, typename T>
    void record(S const& sym, T const& v) {
        mutator_.record(sym, const_cast<T&>(v));
    }
    
    template <typename S, typename T>
    void mstruct(S const& sym, T const& v) {
        mutator_.sequence(sym, const_cast<T&>(v));
    }
};

} // end namespace mo_detail

template <typename T, typename F>
void mo_process_override(T const& value, F& functor)
{
    value.apply(functor);
}

/// Process all record fields using MO dispatcher.
///
/// All MO record fields are processed by functors appropriate methods
/// (leaf, record, sequence). Dispatching is done using mo_traits.

template <typename T, typename F>
void mo_process(T const& value, F& functor)
{
    mo::dispatcher<F> functor_disp(functor);
    using tinfra::mo_process_override;
    mo_process_override(value, functor_disp);
}

/// Mutate record fields using MO dispatcher.
///
/// All MO record fields are processed by appropriate possibly mutating 
/// methods (leaf, record, sequence). Dispatching is done using mo_traits.
template <typename T, typename F>
void mo_mutate(T& value, F& functor)
{
    mo::mutate_helper<F> mutator(functor);
    mo::dispatcher< mo::mutate_helper<F> > functor_disp(mutator);
    using tinfra::mo_process_override;
    mo_process_override(value, functor_disp);
}

/// Process value using MO dispatcher.
///
/// Value is processed by appropriate functor methods
/// (leaf, record, sequence). Dispatching is done using mo_traits.
template <typename S, typename T, typename F>
void process(S const& sym,  T const& value, F& functor)
{
    mo::dispatcher<F> functor_disp(functor);
    functor_disp.dispatch(sym, value);
}

/// Mutate value using MO dispatcher.
///
/// Value is potentially mutated by appropriate functor methods
/// (leaf, record, sequence). Dispatching is done using mo_traits.

template <typename S, typename T, typename F>
void mutate(S const& sym,  T& value, F& functor)
{
    mo::dispatcher<F> functor_disp(functor);
    functor_disp.dispatch(sym, value);
}

/// Declare structure/class manifest template function.
///
/// This macro declares <pre>
///
///   template <typename F> void apply(F& f) const
/// </pre>
/// method where F is template type of dispatcher/functor that
/// processes compound record object.
///
/// Intended use:
/// <pre>struct Foo {
///   int a;
///   std::string b;
///   std::vector<int> c;
///
///   TINFRA_MO_MANIFEST(Foo) {
///      TINFRA_MO_FIELD(a);
///      TINFRA_MO_FIELD(b);
///      TINFRA_MO_FIELD(c);
///   }
/// }</pre>

#define TINFRA_MO_MANIFEST(a)  template <typename F> void apply(F& f) const
#define TINFRA_MO_FIELD(a)    f.dispatch(#a, a)
#define TINFRA_MO_FIELD2(s,a)    f.dispatch(#a, s.a)

// symbol based manifests are deprecated
#define TINFRA_MO_SYMBOL_FIELD(a)    f.dispatch(S::a, a)
    
#define TINFRA_SYMBOL_DECL(a) namespace S { extern tinfra::symbol a; } extern int TINFRA_SYMBOL_DECL_ ## a
#define TINFRA_SYMBOL_IMPL(a) namespace S { tinfra::symbol a(#a); } extern int TINFRA_SYMBOL_IMPL_ ## a

#define TINFRA_MO_IS_RECORD(a) namespace tinfra { template<> struct mo_traits<a>: public tinfra::struct_mo_traits<a> {}; }

} // end namespace tinfra

#endif // tinfra_mo_h_included__

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
