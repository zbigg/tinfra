//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_mo_h_included
#define tinfra_mo_h_included

namespace tinfra {
    
//
// mo_named_value
//

template <typename T>
struct mo_named_value {
    const char* name;
    T           value;
    mo_named_value(const char* name_, T value_)
        : name(name_), value(value_) 
    {}
};

template <typename T>
mo_named_value<T> make_mo_named_value(const char* name, T value) {
    return mo_named_value<T>(name, value);
}

template <typename T>
mo_named_value<T const&> make_mo_named_value2(const char* name, T const& value) {
    return mo_named_value<T const&>(name, value);
}

//
// mo_traits
//
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
template <typename V>
struct mo_traits {
    template <typename V2, typename F>
    static void process(const V2& v, F& f) { 
        f(v);
    }
    
    template <typename V2, typename F>
    static void mutate(V2& v, F& f) { 
        f(v);
    }
    
};

template <typename V>
struct struct_mo_traits {
    template <typename V2, typename F>
    static void process(const V2& v, F& f) { 
        f.mstruct(v);
    }
    
    template <typename V2, typename F>
    static void mutate(V2& v, F& f) { 
        f.mstruct(v);
    }
};

template <typename V>
struct container_mo_traits {
    template <typename V2, typename F>
    static void process(const V2& v, F& f) {
        f.container(v);
    }
    
    
    template <typename V2, typename F>
    static void mutate(V2& v, F& f) { 
        f.container(v);
    }
};

//
//
//

template <typename T>
struct mo_traits < std::vector<T> >: 
    public container_mo_traits< std::vector<T> > { };
    
template <typename T>
struct mo_traits < mo_named_value<T> >: 
    public mo_traits< T > { };

template <typename V, typename F>
void mo_mutate(V& value, F& functor);

namespace mo {

template <typename Functor>
struct dispatcher{
    Functor& f;
    dispatcher(Functor& _f) : f(_f) {}
            
    template <typename V>
    void operator() (const V& v)  { 
        mo_traits<V>::process(v, f); 
    }
    
    template <typename V>
    void operator() (V& v)  { 
        mo_traits<V>::mutate(v, f); 
    }
};

template <typename Functor>
struct mutate_helper {
    Functor& mutator_;
    mutate_helper(Functor& mutator) : mutator_(mutator) {}
    
    template <class V>
    void operator () (const V& v) {
        mutator_(const_cast<V&>(v));
    }
    
    template <typename V>
    void mstruct(V& v) {
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
void process(T const& value, F& functor)
{
    mo::dispatcher<F> functor_disp(functor);
    functor_disp(value);
}

template <typename T, typename F>
void mutate(T& value, F& functor)
{
    mo::dispatcher<F> functor_disp(functor);
    functor_disp(value);
}


#define TINFRA_MO_MANIFEST template <typename F> void apply(F& functor) const

#define TINFRA_MO_NAMED_FIELD(field)    functor(tinfra::make_mo_named_value2(#field, field))

} // end namespace tinfra

#endif // tinfra_mo_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
