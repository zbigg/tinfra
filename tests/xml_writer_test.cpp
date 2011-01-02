#include "tinfra/xml_writer.h"
#include "tinfra/xml_event_buffer.h"

#include <tinfra/test.h> // for test infra
#include <vector>

SUITE(tinfra_xml) {
	
TEST(xml_writer_basic1)
{
	using tinfra::xml_event;
	using tinfra::xml_writer;
	using tinfra::xml_buffer_output_stream;
	
	std::vector<xml_event> result;
	xml_buffer_output_stream< std::vector<xml_event> > out(result);
	{
		xml_writer writer(out);
		writer.start("root").attr("arg", "value").end();
		 
	}
	// <root arg="value"/>
	
	CHECK_EQUAL( 2, result.size() );
	CHECK_EQUAL( xml_event::START_ELEMENT, result[0].type );
	CHECK_EQUAL( "root",        result[0].content );
	CHECK_EQUAL( 1,             result[0].attributes.size() );
	CHECK_EQUAL( "arg",         result[0].attributes[0].name );
	CHECK_EQUAL( "value",       result[0].attributes[0].value );
	
	CHECK_EQUAL( xml_event::END_ELEMENT,   result[1].type );
	CHECK_EQUAL( "root",        result[1].content );
	
	//CHECK_EQUAL( "<root arg="value"/>", materialize_xml(result)); 
}

};
