// XMLPrinter

#ifndef __XMLPrinter_h__
#define __XMLPrinter_h__

#include "Symbol.h"

#include "tinfra_lex.h"

namespace xml {

class XMLSymbolMapping {
public:
    typedef tinfra::Symbol Symbol;
    typedef std::map<Symbol, Symbol> translation_table_t;
    
    template<typename T>
    void map_class_by_traits(Symbol const& xml) { 
        class_name_mapping[tinfra::TypeTraits<T>::symbol()] = xml; 
    }
    void map_class_name(Symbol const& src, Symbol const& xml) {        
        class_name_mapping[src] = xml; 
        class_name_rev_mapping[xml] = src;
    }
    void map_field(Symbol const& src, Symbol const& xml) {
        field_mapping[src] = xml;
        field_rev_mapping[xml] = src;
    }
    Symbol map_class_name(Symbol const& s) const {
        return find_symbol(s, class_name_mapping);
    }
    Symbol map_field(Symbol const& s) const  {
        return find_symbol(s, field_mapping);
    }
    
    Symbol unmap_class_name(Symbol const& s) const {
        return find_symbol(s, class_name_rev_mapping);
    }
    Symbol unmap_field(Symbol const& s) const  {
        return find_symbol(s, field_rev_mapping);
    }
    
private:
    static Symbol find_symbol(Symbol const& s, translation_table_t const& t) {
        if( t.size() == 0 ) return s;
        translation_table_t::const_iterator r = t.find(s);
        if( r == t.end() ) return s;
        return r->second;
    }
    
    static void revert_mapping(translation_table_t const& src, translation_table_t& dest) {
        for(translation_table_t::const_iterator i = src.begin(); i != src.end(); ++i )
            dest[i->second] = i->first;
    }
        
    translation_table_t class_name_mapping;
    translation_table_t class_name_rev_mapping;
    translation_table_t field_mapping;
    translation_table_t field_rev_mapping;
};
    
namespace detail {
    
class XMLPrinterFunctor
{
public:	
	XMLPrinterFunctor(std::ostream& _out, XMLSymbolMapping const& mapping): 
            out(_out), 
            indent(0), 
            in_arg_list(false),
            mapping(mapping) {}
	
        template <typename T>
        void managed_struct(T const& object, const tinfra::Symbol& field_symbol)
        {
            tinfra::Symbol field_tag_symbol = mapping.map_field(field_symbol);            
            begin_composite(field_tag_symbol);
            tinfra::tt_process<T>(object, *this);
            end_composite(field_tag_symbol);
        }
        template <typename T>
        void list_container(T const& container, tinfra::Symbol const& field_symbol)
        {
            static const tinfra::Symbol item_tag_symbol = mapping.map_class_name(tinfra::TypeTraits<typename T::value_type>::symbol());
            tinfra::Symbol field_tag_symbol = mapping.map_field(field_symbol);
            begin_composite(field_tag_symbol);
            for( typename T::const_iterator i = container.begin(); i != container.end(); ++i ) {
                tinfra::TypeTraits<typename T::value_type>::process(*i,item_tag_symbol, *this);
            }
            end_composite(field_tag_symbol);
        }
        
	void begin_composite(const tinfra::Symbol& s) 
        { 
            if( in_arg_list ) {
                out << ">" << endl;
            }
            doindent(indent);
            out << "<" << s.c_str();
            in_arg_list = true;
            indent += 1;
        }	
	void end_composite(const tinfra::Symbol& s) 
        {
            indent -=1;
            if( in_arg_list ) {
                out << "/>" << endl;
                in_arg_list = false;
            } else {
                doindent(indent);
                out << "</"  << s.c_str() << ">" << endl;
            }  
        }
	

	template <typename T>
	void operator () (const tinfra::Symbol& field_symbol, const T& t) {
                out << " " << mapping.map_field(field_symbol).c_str() << "=\"";
                tinfra::to_string(t, out);
                out << "\"";            
	}
private:
	std::ostream& out;
	int           indent;
        bool          in_arg_list;
        XMLSymbolMapping const& mapping;        
        
	void doindent(int indent) {
		for(int i = 0; i < indent; ++i) out << "    ";
	}	
};

} // end xml::detail

struct XMLPrinter {	
    template <typename T>
    static void write(ostream& o, const tinfra::Symbol& s, const T& x)
    {
        XMLSymbolMapping dummy_map;
        detail::XMLPrinterFunctor f(o,dummy_map);
        
        tinfra::TypeTraits<T>::process(x,s,f);
    }

    template <typename T>
    static void write(ostream& o, const tinfra::Symbol& s, const T& x, XMLSymbolMapping const& mapping)
    {
        detail::XMLPrinterFunctor f(o,mapping);
        
        tinfra::TypeTraits<T>::process(x,s,f);
    }
};

} // namespace xml

#endif // __XMLPrinter_h_
