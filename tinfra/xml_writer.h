#ifndef tinfra_xml_writer_h_included
#define tinfra_xml_writer_h_included

#include "xml_stream.h"

#include <tinfra/stream.h>

#include <memory>

namespace tinfra {
    
struct xml_writer_options {
	bool human_readable;
	bool short_string_inline;
	
	int  indentation_size;
	char indentation_character;
	
	/// initialize defaults;
	xml_writer_options();
};
std::auto_ptr<xml_output_stream> xml_stream_writer(tinfra::output_stream* in, xml_writer_options const& options);

} // end namespace tinfra

#endif // tinfra_xml_writer_h_included

