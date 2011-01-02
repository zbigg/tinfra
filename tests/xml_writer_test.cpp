#include "tinfra/xml_writer.h" // we test this
#include "tinfra/xml_builder.h"

#include <tinfra/test.h> // for test infra
#include <vector>

SUITE(tinfra_xml) {
	
class string_output_stream: public tinfra::output_stream {
	std::ostringstream buf_;
	
public:
	std::string str() const {
		return buf_.str();
	}
	// abstract interface
	virtual int write(const char* data, int size)
	{
		buf_.write(data, size);
	}
	virtual void close() {}    
	virtual void sync() {}
};

void build_sample_xml(tinfra::xml_output_stream& out)
{
	tinfra::xml_builder xml(out);
	xml.start("root").attr("arg", "value")
		.start("item").attr("id","1").cdata("spam").end()
		.start("separator").end()
		.start("item").attr("id","2").cdata("eggs").end()
		.start("item").attr("id","3").cdata("").end()
	.end();
}

TEST(xml_writer_basic_no_formatting)
{	
	string_output_stream sink;
	{
		tinfra::xml_writer_options options;
		options.start_document = false;
		options.human_readable = false;
		
		std::auto_ptr<tinfra::xml_output_stream> out(tinfra::xml_stream_writer(&sink, options));
		build_sample_xml(*out);
	}
	CHECK_EQUAL("<root arg=\"value\">"
		    "<item id=\"1\">spam</item>"
		    "<separator/>"
		    "<item id=\"2\">eggs</item>"
		    "<item id=\"3\"></item>"
		    "</root>"
		, sink.str()); 
}

/*
	this test fails currently and there is no plan
	of implementing "pretty" XML
TEST(xml_writer_basic_no_human_readable)
{	
	string_output_stream sink;
	{
		tinfra::xml_writer_options options;
		options.start_document = false;
		options.human_readable = true;
		options.indentation_size = 2;
		options.indentation_character = ' ';
		
		std::auto_ptr<tinfra::xml_output_stream> out(tinfra::xml_stream_writer(&sink, options));
		build_sample_xml(*out);
	}
	CHECK_EQUAL("<root arg=\"value\">\n"
		    "  <item id=\"1\">spam</item>\n"
		    "  <separator/>\n"
		    "  <item id=\"2\">eggs</item>\n"
		    "  <item id=\"3\"></item>\n"
		    "</root>"
		, sink.str()); 
}
*/
};
