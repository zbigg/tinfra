#ifndef tinfra_xml_xml_writer_h_included
#define tinfra_xml_xml_writer_h_included

#include "xml_stream.h"

#include "tinfra/tstring.h"

#include <vector>
#include <string>
#include <utility>
#include <stack>

namespace tinfra {

class xml_writer {
	xml_output_stream& out_;
public:
	xml_writer(xml_output_stream& out): out_(out) {}
	~xml_writer();
	
	void close();

	//  tag attr end method
	xml_writer& start(tstring const& name);
	xml_writer& attr(tstring const& name, tstring const& value);
	xml_writer& end();

	// direct tag writing	
	xml_writer& start_element(tstring const& name, xml_event_arg_list const& args);
	xml_writer& start_element(tstring const& name);
	xml_writer& end_element(tstring const& name);
	
	/// Character data
	xml_writer& cdata(tstring const& name);
	
private:
	enum  {
		NO_CONTENT,
		TAG_OPENED,
		IN_NON_EMPTY_TAG
	} state;
	
	std::string opened_tag;
	std::stack<std::string>    element_stack;
	std::vector<std::pair<std::string, std::string> > current_attrs;
	
	void flush_opened_tag();
};

} // namsepace tinfra

#endif // tinfra_xml_xml_writer_h_included

