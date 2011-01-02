#ifndef tinfra_xml_xml_builder_h_included
#define tinfra_xml_xml_builder_h_included

#include "xml_stream.h"

#include "tinfra/tstring.h"

#include <vector>
#include <string>
#include <utility>
#include <stack>

namespace tinfra {

class xml_builder {
	xml_output_stream& out_;
public:
	xml_builder(xml_output_stream& out): out_(out) {}
	~xml_builder();
	
	void close();

	//  tag attr end method
	xml_builder& start(tstring const& name);
	xml_builder& attr(tstring const& name, tstring const& value);
	xml_builder& end();

	// direct tag writing	
	xml_builder& start_element(tstring const& name, xml_event_arg_list const& args);
	xml_builder& start_element(tstring const& name);
	xml_builder& end_element(tstring const& name);
	
	/// Character data
	xml_builder& cdata(tstring const& name);
	
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

#endif // tinfra_xml_xml_builder_h_included

