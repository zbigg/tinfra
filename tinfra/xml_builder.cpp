#include "xml_builder.h"

#include <cassert>

namespace tinfra {

xml_builder::xml_builder(xml_output_stream& out):
	out_(out),
	state(NO_CONTENT)
{
}
xml_builder::~xml_builder()
{
	close();
}

void xml_builder::close()
{
	// well should i automatically close all
	// tags ?
	// 1st idea no, program should be correct and
	// here i should assert that this->element_stack.size() == 0
	assert( this->element_stack.empty() );
}

//  tag attr end method
xml_builder& xml_builder::start(tstring const& name)
{
	flush_opened_tag();
	this->opened_tag = name;
	state = TAG_OPENED;
	return *this;
}

xml_builder& xml_builder::attr(tstring const& name, tstring const& value)
{
	assert( state == TAG_OPENED );
	this->current_attrs.push_back( std::make_pair( name.str(), value.str() ));
	return *this;
}

xml_builder& xml_builder::end()
{
	flush_opened_tag();
	assert( this->element_stack.size() > 0 );
	
	std::string name = this->element_stack.top();
	
	end_element(name);
	return *this;
}

xml_builder& xml_builder::start_element(tstring const& name, xml_event_arg_list const& args)
{
	flush_opened_tag();
	
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

xml_builder& xml_builder::start_element(tstring const& name)
{
	xml_event_arg_list empty_args;
	return start_element(name, empty_args);
}
	
xml_builder& xml_builder::end_element(tstring const& name)
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

xml_builder& xml_builder::cdata(tstring const& content)
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

void xml_builder::flush_opened_tag()
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
		state = NO_CONTENT;
		start_element( this->opened_tag, args );
		this->opened_tag = "";
		this->current_attrs.clear();
	}
}

} // namsepace tinfra

