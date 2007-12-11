// XMLPrinter

#ifndef __XMLPrinter_h__
#define __XMLPrinter_h__

#include "tinfra/Symbol.h"
#include "tinfra/tinfra_lex.h"
#include "tinfra/xml/XMLStream.h"

namespace tinfra {
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
	XMLPrinterFunctor(XMLOutputStream& _out, XMLSymbolMapping const& mapping): 
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
            out.start_tag(s.c_str());
            /* old
            if( in_arg_list ) {
                out << ">" << endl;
            }
            doindent(indent);
            out << "<" << s.c_str();
            in_arg_list = true;
            indent += 1;
            */
        }	
	void end_composite(const tinfra::Symbol& s) 
        {
            out.end_tag(s);
            /*
            indent -=1;
            if( in_arg_list ) {
                out << "/>" << endl;
                in_arg_list = false;
            } else {
                doindent(indent);
                out << "</"  << s.c_str() << ">" << endl;
            }
            */
        }
	

	template <typename T>
	void operator () (const tinfra::Symbol& field_symbol, const T& t) {
            std::string tmp;
            tinfra::to_string(t, tmp);
            out.arg(mapping.map_field(field_symbol).c_str(), tmp.c_str());
            /*out << " " << mapping.map_field(field_symbol).c_str() << "=\"";
            tinfra::to_string(t, out);
            out << "\"";
            */
	}
private:
	XMLOutputStream& out;
	int           indent;
        bool          in_arg_list;
        XMLSymbolMapping const& mapping;        
        
	void doindent(int indent) {
            //for(int i = 0; i < indent; ++i) out << "    ";
	}
};

} // end xml::detail

template <typename T>
static void dump(XMLOutputStream& o, const tinfra::Symbol& s, const T& x)
{
    XMLSymbolMapping dummy_map;
    detail::XMLPrinterFunctor f(o,dummy_map);
    
    tinfra::TypeTraits<T>::process(x,s,f);
}

template <typename T>
static void dump(XMLOutputStream& o, const tinfra::Symbol& s, const T& x, XMLSymbolMapping const& mapping)
{
    detail::XMLPrinterFunctor f(o,mapping);
    
    tinfra::TypeTraits<T>::process(x,s,f);
}


///
/// XMLStream reader
///

namespace detail {
    
class XMLStreamMutator {
    XMLInputStream& xml_stream;
    Symbol target_tag;
    const XMLSymbolMapping& mapping;
    bool processed;
public:
    XMLStreamMutator(XMLInputStream& xml_stream, Symbol target,xml::XMLSymbolMapping const& mapping): 
        xml_stream(xml_stream), target_tag(target),mapping(mapping),processed(false) {}
    
    template <typename T>
    void managed_struct(T& object, const tinfra::Symbol& field_tag)
    {
        if( field_tag != target_tag ) return;
        processed = true;
        //cerr << "XMLStreamMutator::managed_struct( symbol=" << field_tag.c_str() << " type=" << tinfra::TypeTraits<T>::name() << ")" << endl;
        {
            XMLEvent* e = seek_to_start(mapping.map_field(field_tag).c_str());
            if( !e ) return;
            apply_args<T>(object, e->args());
        }        
        
        XMLEvent* e = xml_stream.peek();
        while(e) {
            if( e == 0 ) return;
            if( e->type == XMLEvent::START_TAG) {
                Symbol subtag_symbol(e->tag_name());
                subtag_symbol = mapping.unmap_field(subtag_symbol);
                XMLStreamMutator subtag_processor(xml_stream, subtag_symbol, mapping);
                tinfra::tt_mutate<T>(object, subtag_processor);
                if( !subtag_processor.processed ) break;
                e = xml_stream.peek();
            } else if( e->type == XMLEvent::END_TAG) {
                e = xml_stream.read();
                break;
            } else if( e->type == XMLEvent::CHAR_DATA) {
                // TODO: don't know how to deal with them so
                e = xml_stream.read(); // ignore
            } else {
                e = xml_stream.read(); // ignore all the rest
            }
        }
        //cerr << " __exit XMLStreamMutator::managed_struct( symbol=" << field_tag.c_str() << ")" << endl;
    }
    template <typename T>
    void list_container(T& container, tinfra::Symbol const& field_tag)
    {   
        if( field_tag != target_tag ) return;
            
        static const Symbol item_symbol = mapping.map_class_name(tinfra::TypeTraits<typename T::value_type>::symbol());
        processed = true;
        //cerr << "XMLStreamMutator::list_container( list ... symbol=" << field_tag.c_str() << " type=" << tinfra::TypeTraits<T>::name() << ")" << endl;        
        {
            XMLEvent* e = seek_to_start(mapping.map_field(field_tag).c_str());
            if( !e ) return;
        }
        
        XMLEvent* e = xml_stream.peek();
        while(e) {
            if( e->type == XMLEvent::START_TAG) {
                Symbol subtag_symbol(e->tag_name());
                subtag_symbol = mapping.unmap_field(subtag_symbol);
                XMLStreamMutator subtag_processor(xml_stream, subtag_symbol, mapping);
                typename T::value_type element;
                tinfra::TypeTraits<typename T::value_type>::mutate(element, subtag_symbol, subtag_processor);
                if( !subtag_processor.processed ) break;
                container.push_back(element);
                e = xml_stream.peek();
            } else if( e->type == XMLEvent::END_TAG) {
                e = xml_stream.read();
                return;
            } else if( e->type == XMLEvent::CHAR_DATA) {
                // TODO: don't know how to deal with them so
                e = xml_stream.read(); // ignore
            } else {
                e = xml_stream.read(); // ignore all the rest
            }
        }
        //cerr << "XMLStreamMutator::list_container( list ... symbol=" << field_tag.c_str() << ")" << endl;        
    }
    
    template<typename T>
    void operator()(tinfra::Symbol const& field_tag, T& target) {
        // XXX is 'a' a TAG or SYMBOL???
        if( field_tag != target_tag) return;
        processed = true;
        
        cerr << "WTF ! XMLStreamMutator::()( symbol=" << field_tag.c_str() << " type=" << tinfra::TypeTraits<T>::name() << ")" << endl;
        if( seek_to_start(field_tag.c_str()) )
            seek_to_end(field_tag.c_str());
    }
    
private:
    XMLEvent* seek_to_start(const char* tag_name)
    {
        //cerr << "XMLStreamMutator::seek_to_start(" << tag_name << ")" << endl;
        while(true) {
            XMLEvent* e = xml_stream.read();
            if( e == 0 ) {
                return 0; // FIXME: it's an error
            }            
            if( e->type == XMLEvent::START_TAG) {
                if( strcmp(e->tag_name(),tag_name) == 0) {
                    return e;
                }                
            } else if( e->type == XMLEvent::END_TAG) {
                //cerr << "XMLStreamMutator::seek_to_start(" << tag_name << ") FAILED END" << endl;
                return 0;
            }
        }
        //cerr << "XMLStreamMutator::seek_to_start(" << tag_name << ") FAILED ???" << endl;
        return 0; // XXX: or throw
    }
    
    XMLEvent* seek_to_end(const char* tag_name)
    {
        //cerr << "XMLStreamMutator::seek_to_end(" << tag_name << ")" << endl;
        while(true) {
            XMLEvent* e = xml_stream.read();
            if( e == 0 ) {
                return 0; // FIXME: it's an error
            }            
            if( e->type == XMLEvent::END_TAG) {                
                if( strcmp(e->tag_name(),tag_name) == 0 ) {
                    //cerr << "XMLStreamMutator::seek_to_end(" << tag_name << ") OK!" << endl;
                    return e;
                }                
            } else if( e->type == XMLEvent::START_TAG) {
                seek_to_end(e->tag_name());
            }
        }
        //cerr << "XMLStreamMutator::seek_to_end(" << tag_name << ") FAILED ???" << endl;
        return 0; // XXX: or throw
    }
    
    template<typename T>
    void apply_args(T& target, const char* const* args)
    {
        const char* const* ia = args;        
        if( !ia ) return;        
        while( *ia ) {
            const char* tag_name = ia[0];
            const char* value = ia[1];
            tinfra::Symbol symbol_name = mapping.unmap_field(tag_name);
            //cerr << "AA:" << tinfra::TypeTraits<T>::name() << "::" << symbol_name.c_str() << "(" << tag_name << ") <= " << value << endl;
            tinfra::lexical_set<T>(target, symbol_name, value);
            ia += 2;
        }
    }
};

} // namespace detail

template <typename T>
void load(XMLInputStream& xml_input, const Symbol& root, T& target, xml::XMLSymbolMapping const& mapping)
{
    detail::XMLStreamMutator subtag_processor(xml_input,  root, mapping);
    tinfra::TypeTraits<T>::mutate(target, root, subtag_processor);

}


} } // namespace tinfra::xml

#endif // __XMLPrinter_h_
