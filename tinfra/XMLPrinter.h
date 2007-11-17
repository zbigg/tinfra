// XMLPrinter

#ifndef __XMLPrinter_h__
#define __XMLPrinter_h__

#include "Symbol.h"

#include "tinfra_lex.h"

namespace xml {

class XMLSymbolMapping {
    typedef tinfra::Symbol Symbol;
    typedef std::map<Symbol, Symbol> translation_table_t;
    
    void map_class_name(Symbol const& src, tinfra,Symbol const& xml) { 
        class_name_mapping[src] = xml; 
    }
    void map_field(Symbol const& src, Symbol const& xml) {
        field_mapping[src] = xml;
    }
    Symbol map_class_name(Symbol const s) {
        return find_symbol(s, class_name_mapping);
    }
    Symbol map_field(Symbol const s) {
        return find_symbol(s, field_mapping);
    }
    
private:
    static Symbol find_symbol(Symbol const& s, translation_table_t const& t) {
        if( t.size() == 0 ) return s;
        translation_table_t::const_iterator r = t.find(other);
        if( r == t.end() ) return s;
        return r->second;
    }
    translation_table_t class_name_mapping;
    translation_table_t field_mapping;    
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
        void managed_struct(T const& object, const tinfra::Symbol& object_symbol)
        {
            Symbol xml_class_symbol = symbol_mapping.map_class_name(object_symbol);
            begin_composite(xml_class_symbol);
            tinfra::tt_process<T>(object, *this);
            end_composite(xml_class_symbol);
        }
        template <typename T>
        void list_container(T const& container, tinfra::Symbol const& container_symbol, tinfra::Symbol const& item_symbol)
        {
            Symbol xml_class_symbol = symbol_mapping.map_class_name(container_symbol);
            begin_composite(xml_class_symbol);
            for( typename T::const_iterator i = container.begin(); i != container.end(); ++i ) {
                tinfra::TypeTraits<typename T::value_type>::process(*i,item_symbol, *this);
            }
            end_composite(xml_class_symbol);
        }
        
	void begin_composite(const tinfra::Symbol& s) 
        { 
            if( in_arg_list ) {
                out << ">" << endl;
            }
            doindent(indent);
            out << "<" << map(s).c_str();
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
                out << "</"  << map(s).c_str() << ">" << endl;
            }  
        }
	

	template <typename T>
	void operator () (const tinfra::Symbol& s, const T& t) {
                out << " " << symbol_mapping.map_field(s).c_str() << "=\"";
                std::string a;
                tinfra::to_string(t, a);
                out << a.c_str();
                out << "\"";            
	}
private:
	std::ostream& out;
	int           indent;
        bool          in_arg_list;
        XMLSymbolMappingconst& symbol_mapping;        
        
	void doindent(int indent) {
		for(int i = 0; i < indent; ++i) out << "    ";
	}	
};

} // end xml::detail

struct XMLPrinter {	
    template <typename T>
    static void write(ostream& o, const tinfra::Symbol& s, const T& x)
    {
        symbol_mapping dummy_map;
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
