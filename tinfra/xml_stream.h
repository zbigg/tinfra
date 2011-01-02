#ifndef tinfra_xml_stream_h_included
#define tinfra_xml_stream_h_included

#include <tinfra/tstring.h>

#include <vector>

namespace tinfra {

struct xml_event_arg {
    tstring name;
    tstring value;
};

typedef std::vector<xml_event_arg> xml_event_arg_list;

struct xml_event {
    enum xml_event_type { 
        END,
        START_ELEMENT,
        END_ELEMENT,
        CDATA
    };
    
    xml_event_type type;
    
    tstring            content;
    xml_event_arg_list attributes;
};

struct xml_input_stream {
    virtual ~xml_input_stream() {}
    
    virtual xml_event read() = 0;
};

struct xml_output_stream {
    virtual ~xml_output_stream() {}
    
    virtual void write(xml_event const& ev) = 0;
};


} // end namespace tinfra

#endif // tinfra_xml_stream_h_included
