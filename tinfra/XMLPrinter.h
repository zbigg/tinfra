// XMLPrinter

#ifndef __XMLPrinter_h__
#define __XMLPrinter_h__

#include "Symbol.h"

#include "tinfra_lex.h"

namespace xml {

typedef std::map<tinfra::Symbol, tinfra::Symbol> symbol_mapping;
    
namespace detail {
    
class XMLPrinterFunctor
{
public:	
	XMLPrinterFunctor(std::ostream& _out, symbol_mapping const& mapping): 
            out(_out), 
            indent(0), 
            in_arg_list(false),
            mapping(mapping) {}
	
	void begin_composite(const tinfra::Symbol& s, tinfra::CompositeType) 
        { 
            if( in_arg_list ) {
                out << ">" << endl;
            }
            doindent(indent);
            out << "<" << map(s).c_str();
            in_arg_list = true;
            indent += 1;
        }	
	void end_composite(const tinfra::Symbol& s, tinfra::CompositeType) 
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
            out << " " << map(s).c_str() << "=\"";
            out << t;
            out << "\"";            
	}
private:
	std::ostream& out;
	int           indent;
        bool          in_arg_list;
        symbol_mapping const& mapping;
        tinfra::Symbol map(tinfra::Symbol const& other) {
            if( mapping.size() == 0 ) return other;
            symbol_mapping::const_iterator r = mapping.find(other);
            if( r == mapping.end() ) return other;
            return r->second;
        }
        
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
    static void write(ostream& o, const tinfra::Symbol& s, const T& x, symbol_mapping const& mapping)
    {
        detail::XMLPrinterFunctor f(o,mapping);
        
        tinfra::TypeTraits<T>::process(x,s,f);
    }
};

} // namespace xml

#endif // __XMLPrinter_h_
