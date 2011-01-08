//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <iostream>
#include <expat.h>
#include <fstream>

#include "tinfra/xml/XMLStream.h"

namespace tinfra {
namespace xml {
namespace detail {

class XMLStreamReader {
    
    XML_Parser parser;
    XMLOutputStream& target;
    
public:
    XMLStreamReader(XMLOutputStream& target): target(target) 
    {
        initParser();
    }

    void initParser()
    {
        parser = XML_ParserCreate("UTF-8");        
        XML_SetStartElementHandler(parser, &XMLStreamReader::startElementHandler);
        XML_SetEndElementHandler(parser, &XMLStreamReader::endElementHandler);
        XML_SetCharacterDataHandler(parser, &XMLStreamReader::characterDataHandler);
        XML_SetUserData(parser, this);
    }

    bool parse(std::streambuf& in)
    {
        while( true) {
            int BUFF_SIZE = 16384;
            
            char *buff = (char*)XML_GetBuffer(parser, BUFF_SIZE);
            if (buff == NULL) {
                // TODO: throw something
                //cerr << "A" << endl; 
                return false;
            }

            int bytes_read = in.sgetn(buff, BUFF_SIZE);
            if (bytes_read < 0) {
                // TODO: throw something
                //cerr << "B" << endl; 
                return false;
            }

            if (! XML_ParseBuffer(parser, bytes_read, bytes_read == 0)) {
                //cerr << "XML parse error: " << XML_ErrorString(XML_GetErrorCode(parser)) << 
                //        " at " << XML_GetCurrentLineNumber(parser) << ":" << XML_GetCurrentColumnNumber(parser) << endl;
                // TODO: throw something
                return false;
            }

            if (bytes_read == 0) return true;
        }
    }
    
    ~XMLStreamReader()
    {
        XML_ParserFree(parser);
    }
    
    void startElement(const char* name, const char** attributes)
    {
        target.start_tag(name, attributes);
    }
    void endElement(const char* name)
    {
        target.end_tag(name);
    }
    void characterData(const char* data, int)
    {
        target.char_data(data);
    }
    static void XMLCALL startElementHandler(void *userData,
                                   const XML_Char *name,
                                   const XML_Char **atts)
    {
        ((XMLStreamReader*)userData)->startElement(name, atts);
    }
    
    static void XMLCALL endElementHandler(void *userData, const XML_Char *name)
    {
        ((XMLStreamReader*)userData)->endElement(name);
    }
    
    static void XMLCALL characterDataHandler(void *userData, const XML_Char *s, int len)
    {
        ((XMLStreamReader*)userData)->characterData(s,len);
    }
};

} // end namespace detail
void read_xml(std::streambuf& in, XMLOutputStream& sink)
{
    detail::XMLStreamReader reader(sink);
    reader.parse(in);
}

void read_xml(const char* path, XMLOutputStream& sink)
{
    std::filebuf in;
    in.open(path,std::ios::in);
    if( !in.is_open() ) {
        //cerr << "can't read file: " << filename << endl;
        // TODO: throw something
        return;
    }   
    read_xml(in,sink);
}

} } // end namespace tinfra::xml
