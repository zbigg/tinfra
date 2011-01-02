#include "xml_writer.h"

#include <cassert>

namespace tinfra {

xml_writer::~xml_writer()
{
	close();
}

void xml_writer::close()
{
	// well should i automatically close all
	// tags ?
	// 1st idea no, program should be correct and
	// here i should assert that this->element_stack.size() == 0
	assert( this->element_stack.empty() );
}

//  tag attr end method
xml_writer& xml_writer::start(tstring const& name)
{
	flush_opened_tag();
	this->opened_tag = name;
	state = TAG_OPENED;
	return *this;
}

xml_writer& xml_writer::attr(tstring const& name, tstring const& value)
{
	assert( state == TAG_OPENED );
	this->current_attrs.push_back( std::make_pair( name.str(), value.str() ));
	return *this;
}

xml_writer& xml_writer::end()
{
	flush_opened_tag();
	assert( this->element_stack.size() > 0 );
	
	std::string name = this->element_stack.top();
	
	end_element(name);
	return *this;
}

xml_writer& xml_writer::start_element(tstring const& name, xml_event_arg_list const& args)
{
	xml_event ev;
	ev.type = xml_event::START_ELEMENT;
	ev.content = name;
	ev.attributes = args;
	out_.write(ev);
	
	// setup internal state, we've inside new, currently empty tag
	state = NO_CONTENT;
	this->element_stack.push(name.str());
	
	return *this;
}

xml_writer& xml_writer::start_element(tstring const& name)
{
	xml_event_arg_list empty_args;
	return start_element(name, empty_args);
}
	
xml_writer& xml_writer::end_element(tstring const& name)
{
	assert( this->element_stack.size() > 0 );
	assert( name == this->element_stack.top());
	
	{
		xml_event ev;
		ev.type = xml_event::END_ELEMENT;
		ev.content = name;
		out_.write(ev);
	}
	
	state = IN_NON_EMPTY_TAG;
	this->element_stack.pop();
	
	return *this;
}

xml_writer& xml_writer::cdata(tstring const& content)
{
	flush_opened_tag();
	
	{
		xml_event ev;
		ev.type = xml_event::CDATA;
		ev.content = content;
		out_.write(ev);
	}
	
	state = IN_NON_EMPTY_TAG;
	return *this;
}

void xml_writer::flush_opened_tag()
{
	if( state == TAG_OPENED ) {
		// convert into xml_event_arg_list and
		xml_event_arg_list args;
		args.reserve( this->current_attrs.size() );
		for( int i = 0; i < this->current_attrs.size(); ++i ) {
			xml_event_arg arg;
			arg.name = this->current_attrs[i].first;
			arg.value = this->current_attrs[i].second;
			args.push_back(arg); 
		}
		// officially flush
		start_element( this->opened_tag, args );
		
		current_attrs.clear();
		state = NO_CONTENT;
	}
}

} // namsepace tinfra

