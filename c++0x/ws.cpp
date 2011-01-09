//#include <tinfra/mo.h>

#include <iostream>
#include <typeinfo>
#include <vector>
#include <string>
#include <functional>

#include <tinfra/mo.h>
#include <tinfra/xml_builder.h>
#include <tinfra/xml_writer.h>
#include <tinfra/fmt.h>

#include <memory>

struct Book {
	std::string Title;
	std::string Author;
	std::string Date;
	std::string ISBN;
	std::string Publisher;
	
	TINFRA_MO_MANIFEST(Book) {
		TINFRA_MO_FIELD(Title);
		TINFRA_MO_FIELD(Author);
		TINFRA_MO_FIELD(Date);
		TINFRA_MO_FIELD(ISBN);
		TINFRA_MO_FIELD(Publisher);
	}
};

namespace tinfra {

template <>
struct mo_traits<Book>: public struct_mo_traits<Book> { };

};

struct xsd_processor {
	tinfra::xml_builder& xml;
	xsd_processor(tinfra::xml_builder& xml_): xml(xml_) {}
	
	/*
	template <typename T>
	void leaf(const char* name, optional<T> const&) 
	{
		leaf_int(name, "xsd:integer");
	}
	*/
	
	void leaf(const char* name, int const&) 
	{
		leaf_int(name, "xsd:integer");
	}
	
	void leaf(const char* name, std::string const&) 
	{
		leaf_int(name, "xsd:string");
	}
	
	void leaf_int(const char* name, const char* type)
	{
		xml.start("xsd:element").
		    attr("name", name).
		    attr("type", type).end();
	}
	
	/*
	template <typename T>
	void leaf(const char* name, optional<T> const&) 
	{
		leaf_int(name, "xsd:integer");
		add minoccurs 0, maxoccurs 1
	}
	*/
    
	template <typename T>
	void record(const char* name, T const& v)
	{
		xml.start("xsd:complexType").attr("name", name);
		xml.start("xsd:sequence");
		tinfra::mo_process(v, *this);
		xml.end().end();
	}
	
	/*
	template <typename S, typename T>
	void sequence(S sym, T const& v)
	{
		//xml_.start("complexType").name(name);
	}
	*/
};

int getPrice(Book const& book, int from)
{
}

using tinfra::fmt;

std::string tns(tinfra::tstring const& str)
{
	using tinfra::fmt;
	return fmt("tns:%s") % str;
}

void xsd_for_param(const char* name, const char* type, tinfra::xml_builder& xml)
{
	xml.start("xsd:element").
	    attr("name", name).
	    attr("type", type).end();
}

/*
template<std::size_t idx, typename... types>
struct xsd_for_arguments_helper; 

template<size_t idx>
struct xsd_for_arguments_helper<idx>
{ 
	static void print(tinfra::xml_builder& xml) {}
};

template <size_t idx, typename Head, typename ... Tail>
struct xsd_for_arguments_helper<idx, Head, Tail...> 
{
	static void print(tinfra::xml_builder& xml) {
		
		//std::cout  << idx << " " << typeid(Head).name() << "\n";
		xsd_for_param(fmt("arg%i")%idx, "xsd:int", xml);
		xsd_for_arguments_helper<idx+1, Tail...>::print(xml);
	}
};
*/

#include "type_visitor.h"

struct xsd_argument_processor {
	
	tinfra::xml_builder& xml;
	
	xsd_argument_processor(tinfra::xml_builder& x): xml(x) {}
	
	template <typename T>
	void visit(int idx)
	{
		xsd_for_param(fmt("arg%i")% idx, "xsd:int", xml);
	}
};

template <typename Return, typename... Arguments>
void xsd_for_function(const char* name, Return func (Arguments...), tinfra::xml_builder& xml)
{
	using tinfra::fmt;
	
	xml.start("xsd:element").
	    attr("name", name).
	    attr("type", tns(name)).end();
	xml.start("xsd:element").
	    attr("name", fmt("%sResponse") % name).
	    attr("type", fmt("tns::%sResponse") % name).end();
	    
	xml.start("xsd:complexType").
	    attr("name",name).
	    start("xsd:sequence");
	    
	xsd_argument_processor xap(xml);
	visit_types<xsd_argument_processor, Arguments...>(xap);
	//xsd_for_arguments_helper<0, Arguments...>::print(xml);
	xml.end().end();
	
	
	xml.start("xsd:complexType").
	    attr("name",fmt("%sResponse") % name).
	    start("xsd:sequence");
	xsd_for_param("return", "xsd:int", xml); 
	xml.end().end();
}

template <typename T>
void xsd_for_struct(const char* name, tinfra::xml_builder& xml)
{
	xsd_processor processor(xml);
	T dummy;
	tinfra::process(name, dummy, processor);
}

int main()
{
	tinfra::xml_writer_options writer_options;
	std::auto_ptr<tinfra::xml_output_stream> writer(tinfra::xml_stream_writer(&tinfra::out,writer_options));

	tinfra::xml_builder xml(*writer);
	xml.start("definitions").
	    attr("targetNamespace", "http://reddix.pl/").
	    attr("name", "WSSampleCpp").
	    attr("xmlns", "http://schemas.xmlsoap.org/wsdl/").
	    attr("xmlns:tns", "http://reddix.pl").
	    attr("xmlns:xsd", "http://www.w3.org/2001/XMLSchema").
	    attr("xmlns:soap", "http://schemas.xmlsoap.org/wsdl/soap/");
	    
	{
		xml.start("xsd:schema");
		xsd_for_struct<Book>("book", xml);
		xsd_for_function("getPrice", getPrice, xml);
		xml.end();
	}
	xml.end();
}


