//#include "xml-config.h"

#include "xml_parser.h"

#include <tinfra/tstring.h>

#include <expat.h>
#include <deque>
#include <stdexcept>

namespace tinfra {

class xml_stream_reader_expat: public xml_input_stream {
    tinfra::input_stream* input_;
    XML_Parser parser;
    std::deque<xml_event> queue;
    string_pool pool;
public:    
    xml_stream_reader_expat(tinfra::input_stream* input):
        input_(input)
    {
        parser = XML_ParserCreate("UTF-8");        
        XML_SetStartElementHandler(parser, &xml_stream_reader_expat::startElementHandler);
        XML_SetEndElementHandler(parser, &xml_stream_reader_expat::endElementHandler);
        XML_SetCharacterDataHandler(parser, &xml_stream_reader_expat::characterDataHandler);
        XML_SetUserData(parser, this);
    }
    
    ~xml_stream_reader_expat() {
        XML_ParserFree(parser);
    }
    
    //
    // implementation of xml_input_stream IF
    //
    virtual xml_event read() {
        if( queue.empty() ) 
            parse();
        
        xml_event result = queue.front();
        queue.pop_front();
        return result;
    };
    
    // 
    // parser main
    //
    
    void parse() {
        queue.clear();
        pool.clear();
        
        while( queue.empty() ) {
            int BUFF_SIZE = 16384;
            
            char *buff = (char*)XML_GetBuffer(parser, BUFF_SIZE);
            if (buff == NULL) {
                throw std::runtime_error("XML_GetBuffer");
            }
            
            int bytes_read = input_->read(buff, BUFF_SIZE);
            int is_final = (bytes_read == 0);
            
            if (! XML_ParseBuffer(parser, bytes_read, is_final)) {
                handle_parse_errors();
            }
            
            if( is_final ) {
                xml_event eof;
                eof.type = xml_event::END;
                queue.push_back(eof);
                return;
            }
        }
    }
    void handle_parse_errors()
    {
        throw std::runtime_error("XML_ParseBuffer");
    }
    
    static int attributes_count(const char** attributes)
    {
        const char** i = attributes;
        int result = 0;
        while( *i ) {
            i+=2;
            result++;
        }
        return result;
    }
    //
    // handlers - instance methods
    //
    void startElement(const char* name, const char** attributes)
    {
        xml_event event;
        
        event.type = xml_event::START_ELEMENT;
        event.content = pool.alloc(name);
        
        int count = attributes_count(attributes);
        event.attributes.reserve(count);
        
        for(int i = 0; i < count ; ++i ) {
            const char* name =  attributes[i*2];
            const char* value = attributes[i*2+1];
            event.attributes[i].name = pool.alloc(name);
            event.attributes[i].value = pool.alloc(value);
        }
        queue.push_back(event);
    }
    
    void endElement(const char* name)
    {
        xml_event event;
        
        event.type = xml_event::END_ELEMENT;
        event.content = pool.alloc(name);
        
        queue.push_back(event);
    }
    
    void characterData(const char* data, int len)
    {
        xml_event event;
        
        event.type = xml_event::CDATA;
        event.content = pool.alloc(tstring(data,len));
        
        queue.push_back(event);
    }
    
    //
    // implementation of expat handlers; statics as expat is C callback interface
    //
    static void XMLCALL startElementHandler(void *userData,
                                   const XML_Char *name,
                                   const XML_Char **atts)
    {
        ((xml_stream_reader_expat*)userData)->startElement(name, atts);
    }
    
    static void XMLCALL endElementHandler(void *userData, const XML_Char *name)
    {
        ((xml_stream_reader_expat*)userData)->endElement(name);
    }
    
    static void XMLCALL characterDataHandler(void *userData, const XML_Char *s, int len)
    {
        ((xml_stream_reader_expat*)userData)->characterData(s,len);
    }
};

std::auto_ptr<xml_input_stream> xml_stream_reader(tinfra::input_stream* in)
{
    return std::auto_ptr<xml_input_stream>(new xml_stream_reader_expat(in));
}

} // end namespace tinfra

