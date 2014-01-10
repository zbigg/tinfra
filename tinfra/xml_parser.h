#ifndef tinfra_xml_parser_h_included
#define tinfra_xml_parser_h_included

#include "xml_stream.h"

#include <tinfra/stream.h>

#include <memory>

namespace tinfra {
    
std::auto_ptr<xml_input_stream> xml_stream_reader(tinfra::input_stream* in);

} // end namespace tinfra

#endif // tinfra_xml_parser_h_included
