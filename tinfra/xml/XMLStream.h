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
    
    XMLEvent()
    {
        _args = 0;
    }
    XMLEvent(XMLEvent const& other)
        : type(other.type), _tag_name(other._tag_name), _content(other._content), _args(0)
    {
        // TODO: copy args!
        _args = 0;
    }
    ~XMLEvent() {
        if( _args ) {
            char** ia = _args;
            while( *ia ) {
                free(ia[0]); // name
                free(ia[1]); // value
                ia += 2;
            }
            delete[] _args;
        }
    }
    
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
    
    void report()
    {
        //cerr << "XMLEvent " << type << " name " << _tag_name.c_str() << endl;
    }
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

private:
    void    ensure_tag_start_closed();

    std::streambuf&  dest_;
    unsigned         tag_nest_;
    bool             in_start_tag_;
};

class XMLMemoryStream: public XMLInputStream, public XMLOutputStream {
public:
    
    XMLMemoryStream();
    virtual ~XMLMemoryStream();
    ///
    /// XMLInputStream implementation
    ///
    virtual XMLEvent* peek();
    virtual XMLEvent* read();
    
    ///
    /// XMLOutputStream implementation
    ///    
    virtual void write(XMLEvent const* ev);

    virtual void start_tag(const char* tag_name, const char* const* args = 0);
    virtual void arg(const char* name, const char* value);
    virtual void char_data(const char* value);
    virtual void end_tag(const char* tag_name);
    
private:
    std::vector<XMLEvent*> events;
    int read_cursor;
    int write_cursor;
};
  
/**
    Read XML document.

    Data is read from in and put in sink using interface methods.
*/

void read_xml(std::streambuf& in, XMLOutputStream& sink);
void read_xml(const char* path,   XMLOutputStream& sink);

} }

#endif
