//
// Copyright 2009 (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_structure_printer_h_included_
#define tinfra_structure_printer_h_included_

#include "tinfra/symbol.h"
#include "tinfra/mo.h"

namespace tinfra {
/// Structured value printer
///
/// This is MO functor that will accept any MO compatible value (leafs, 
/// structs and containers) and print them to some stream.
///
/// Generic std::ostream << (stream, value) operator is used.
/// JSON like structure markers are used: 
///  * { } for structures and maps
///  * [] for arrays
///
/// No escaping of values is used for now. This is not reversible conversion.
///
class structure_printer {
    std::ostream& out;
    bool need_separator;
    int  indent_size;
    int  indent_level;
    bool multiline;
    bool showing_name;
    std::vector<bool> sn_history;
public:
    structure_printer(std::ostream& o): 
        out(o), 
        need_separator(false),
        indent_size(0),
        indent_level(0), 
        multiline(false),
        showing_name(true)
    {}
    // customization
    
    void set_multiline(bool is_multine, int new_indenation_size = 4)
    {
        this->multiline = is_multine;
        this->indent_size = new_indenation_size;
    }
    
    // MO contract
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
    void mstruct(tinfra::symbol const& sym, T const& v)
    {
        separate();
        enter(sym, '{');
        push_showing_name(true);
        
        tinfra::mo_process(v, *this);
        
        pop_showing_name();
        need_separator = false;
        separate();
        exit(sym,'}');
    }
    
    template <typename T>
    void container(tinfra::symbol const& sym, T const& v)
    {
        separate();
        enter(sym, '[');
        push_showing_name(false);
        
        typedef typename T::const_iterator  iterator;
        for( iterator i = v.begin(); i != v.end(); ++i ) {
            tinfra::process(symbol(0),*i, *this);
        }
        
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

/*
TODO:
Idea is that, structure_printer should reference (template or dynamic)
strategy that has does real output and formatting. this would allow
generic infra for printing c++ like / json / yaml / xml using same 
structure_printer.

CONS. Probably it would make structure_printer very complicated. Should
first implement at least 3 printers - JSON&xml and decide there is
real common part that can be detached.

class structure_printer_strategy {
    
};
*/
} // end namespace tinfra

#endif // tinfra_structure_printer_h_included_

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
