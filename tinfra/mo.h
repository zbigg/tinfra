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
        f.begin_mo(s, v);
        mo_process(v, f);
        f.end_mo(s, v);
    }
    
    template <typename Functor>
    static void mutate(const symbol& s, T& v, Functor& f) { 
        f.begin_mo(s, v);
        mo_mutate(v, f);
        f.end_mo(s, v);
    }
};

template <typename T>
struct container_mo_traits{
    template <typename Functor>
    static void process(const symbol& s, const T& v, Functor& f);
    
    /*
    template <typename Functor>
    static void mutate(const symbol& s, T& v, Functor& f) { 
        f.begin_container(s, v);
        typedef typename T::value_type value_type;
        typedef typename T::const_iterator  iterator;
        while( f.hasNext() ) {
            v.push_back(f.next());
        }
        f.end_container(s, v);
    }
    */
};

template <typename T>
struct mo_traits < std::vector<T> >: 
    public container_mo_traits< std::vector<T> > { };

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

template <typename T>
template <typename Functor>
void container_mo_traits<T>::process(const symbol& s, const T& v, Functor& f) { 
    f.begin_container(s, v);
    typedef typename T::const_iterator  iterator;
    for( iterator i = v.begin(); i != v.end(); ++i ) {
        tinfra::process(symbol(0),*i, f);
    }
    f.end_container(s, v);
}
#define TINFRA_MO_MANIFEST(a)  template <typename F> void apply(F& f) const

#define TINFRA_MO_FIELD(a)    f(S::a, a)
    
#define TINFRA_SYMBOL_DECL(a) namespace S { extern tinfra::symbol a; }
#define TINFRA_SYMBOL_IMPL(a) namespace S { tinfra::symbol a(#a); }

class field_printer {
    std::ostream& out;
    bool need_separator;
    int  indent_size;
    int  indent_level;
    bool multiline;
    bool showing_name;
    std::vector<bool> sn_history;
public:
    field_printer(std::ostream& o): 
        out(o), 
        need_separator(false),
        indent_size(2),
        indent_level(0), 
        multiline(false),
        showing_name(true)
    {}
    
    template <class T>
    void operator () (const tinfra::symbol& sym, T const& t) 
    {
        separate();
        apply_indent();
        name(sym);
        out << t;
        need_separator = true;
    }
    
    template <typename T>
    void begin_mo(tinfra::symbol const& sym, T const& v)
    {
        separate();
        enter(sym, '{');
        push_showing_name(true);
    }
    
    template <typename T>
    void end_mo(tinfra::symbol const& sym, T const& v)
    {
        pop_showing_name();
        need_separator = false;
        separate();
        exit(sym,'}');
    }
    
    template <typename T>
    void begin_container(tinfra::symbol const& sym, T const& v)
    {
        separate();
        enter(sym, '[');
        push_showing_name(false);        
    }
    
    template <typename T>
    void end_container(tinfra::symbol const& sym, T const& v)
    {
        pop_showing_name();
        need_separator = false;
        separate();
        exit(sym,']');
    }
private:
    void push_showing_name(bool new_value) {
        sn_history.push_back(showing_name);
        showing_name = new_value;
    }
    void pop_showing_name() {
        showing_name = sn_history.at(sn_history.size()-1);
        sn_history.erase(sn_history.end()-1);
    }
    void name(symbol const& sym) {
        if( showing_name ) {
            out << sym << '=';
        }
    }
    void separate() {
        if( need_separator ) {
            out << ", ";
        }
        if( multiline ) {
            out << "\n";
        }
        need_separator = false;
    }
    void apply_indent() {
        if( indenting() ) {
            for(int i = 0; i < indent_level*indent_size; ++i )
                out << " ";
        }
    }
    void enter(tinfra::symbol const& sym, char sep) {
        apply_indent();
        name(sym);
        out << sep;
        indent_level+=1;
        if( !multiline )
            out << " ";
        need_separator = false;
    }
    
    void exit(tinfra::symbol const&, char sep) {
        indent_level -= 1;
        apply_indent();
        if( !multiline )
            out << ' ';
        out << sep;
        need_separator = true;
    }
    
    bool indenting() const { return multiline && indent_level != 0 && indent_size > 0; }
}; 

} // end namespace tinfra

#endif // tinfra_struct_h_included__
