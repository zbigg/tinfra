#ifndef __tinfra_xml_XMLStream_h_h
#define __tinfra_xml_XMLStream_h_h


#include <string>
#include <vector>
#include <streambuf>

namespace tinfra {
namespace xml {

struct XMLEvent {
    enum event_type {
        START_TAG = 0,
        END_TAG = 1,
        CHAR_DATA = 2
    };
    
    XMLEvent();
    XMLEvent(XMLEvent const& other);    
    ~XMLEvent();
    
    const char* tag_name() const { 
        return _tag_name.c_str(); 
    }
    
    const char* content() const { return 
        _content.c_str(); 
    }
    
    const char* const* args() const { 
        return (const char* const*)_args; 
    }
    
    event_type type;
    std::string _tag_name;
    std::string _content;
    char** _args;
};


class XMLInputStream {
public:
    virtual XMLEvent* peek() = 0;
    virtual XMLEvent* read() = 0;
    
    // force virtual destruction
    virtual ~XMLInputStream() {}
};

class XMLOutputStream {    
public:
    virtual void write(XMLEvent const* ev);

    virtual void start_tag(const char* tag_name, const char* const* args = 0) = 0;
    virtual void arg(const char* name, const char* value) = 0;
    virtual void char_data(const char* value) = 0;
    virtual void end_tag(const char* tag_name) = 0;

    // force virtual destruction
    virtual ~XMLOutputStream() {}
};

class FileXMLOutputStream: public XMLOutputStream {
public:
    FileXMLOutputStream(std::streambuf& dest);
    ~FileXMLOutputStream();
    
    virtual void start_tag(const char* tag_name, const char* const* args = 0);
    virtual void arg(const char* name, const char* value);
    virtual void char_data(const char* value);
    virtual void end_tag(const char* tag_name);

    void set_human_readable(bool b) { human_readable_ = b; }
    void start_document();
private:
    void    ensure_tag_start_closed();
    void    maybe_newline();
    void    maybe_indent();
    std::streambuf&  dest_;
    unsigned         tag_nest_;
    bool             in_start_tag_;
    bool             human_readable_;
    bool             xml_document_started_;
};

struct XMLEventBuffer {
    std::vector<XMLEvent*> buffer;
    
    ~XMLEventBuffer();
};

class XMLBufferOutputStream: public XMLOutputStream {
public:    
    XMLBufferOutputStream(XMLEventBuffer& dest);    
    ///
    /// XMLOutputStream implementation
    ///    
    virtual void write(XMLEvent const* ev);

    virtual void start_tag(const char* tag_name, const char* const* args = 0);
    virtual void arg(const char* name, const char* value);
    virtual void char_data(const char* value);
    virtual void end_tag(const char* tag_name);
    
private:
    
    XMLEventBuffer& events;
};

class XMLBufferInputStream: public XMLInputStream {
    
public:
    XMLBufferInputStream(XMLEventBuffer const& pevents);
    ///
    /// XMLInputStream implementation
    ///    
    virtual XMLEvent* read();
    virtual XMLEvent* peek();
private:
    unsigned cursor;
    XMLEventBuffer const& events;
};
  
/**
    Read XML document.

    Data is read from in and put in sink using interface methods.
*/

void read_xml(std::streambuf& in, XMLOutputStream& sink);
void read_xml(const char* path,   XMLOutputStream& sink);

} }

#endif
