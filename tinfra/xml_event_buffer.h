#ifndef tinfra_xml_xml_event_buffer_h_included
#define tinfra_xml_xml_event_buffer_h_included

#include "xml_stream.h"

namespace tinfra {

template <typename Container>
class xml_buffer_input_stream: public xml_input_stream {
	Container const& in_;
	typename Container::const_iterator iter_; 
public:
	xml_buffer_input_stream(Container const& in): 
		in_(in),
		iter_(in_.begin())
	{}
	
	virtual xml_event read()
	{
		if( this->iter_ == this->in_.end() ) {
			xml_event ev;
			ev.type = xml_event::END;
			return ev;
		}
		return *this->iter_++;
	}
};

template <typename Container>
class xml_buffer_output_stream: public xml_output_stream {
	Container& out_;
public:
	xml_buffer_output_stream(Container& out): out_(out) {}
	virtual void write(xml_event const& ev)
	{
		this->out_.push_back(ev);
	}
};

} // end namespace tinfra

#endif // tinfra_xml_xml_event_buffer_h_included

