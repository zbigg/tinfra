#include <iostream>

#include "tinfra/xml/XMLStream.h"

namespace tinfra { 
namespace xml {
    
    
///
/// XMLEvent
///

static void args_array_delete(char** args)
{
    if( args ) {
        char** ia = args;
        while( *ia ) {
            ::free(ia[0]); // name
            ::free(ia[1]); // value
            ia += 2;
        }
        delete[] args;
    }
}
static char** args_array_copy(const char* const* src)
{
    if( src == 0 ) {
        return 0;
    }
    int n = 0;
    {
        const char* const* ia = src;
        while( *ia ) {
            ia += 2;
            n++;
        }
    }
    {
        char** result = new char*[n*2+1];
        char** idest = result;
        const char* const* isrc  = src;
        while( *isrc ) {
            idest[0] = strdup(isrc[0]);
            idest[1] = strdup(isrc[1]);
            //cout << idest[0] << "=" << idest[1] << " ";
            idest += 2;
            isrc  += 2;
        }
        idest[0] = 0;
        return result;
    }
}

XMLEvent::XMLEvent()
{
    _args = 0;
}
XMLEvent::XMLEvent(XMLEvent const& other)
    : type(other.type), _tag_name(other._tag_name), _content(other._content), _args(0)
{
    // TODO: copy args!
    _args = args_array_copy(other._args);
}
XMLEvent::~XMLEvent() {
    args_array_delete(_args);
}

void event_report(const char* what, XMLEvent const& ev, std::ostream& o)
{
    //if( ev.type == XMLEvent::CHAR_DATA ) return;
    o << what << ": XMLEvent " << ev.type;
    if( ev.type == XMLEvent::CHAR_DATA ) {
        o << " chardata";  
    } if( ev.type == 0 ) {
        o << " start tag: " << ev._tag_name.c_str();
        if( ev._args )
            o << " (with args)";
    } else 
        o << "   end tag: " << ev._tag_name.c_str();
    o << std::endl;
}
///
/// XMLOutputStream::
///


void XMLOutputStream::write(XMLEvent const* ev)
{        
    XMLEvent::event_type type = ev->type;
    if( type == XMLEvent::START_TAG ) {
        start_tag(ev->tag_name(), ev->args());
    } else if( type == XMLEvent::END_TAG) {
        end_tag(ev->tag_name());
    } else if( type == XMLEvent::CHAR_DATA ) {
        char_data(ev->content());
    } else {
        //I(false);
    }
}

///
/// FileXMLOutputStream
///

FileXMLOutputStream::FileXMLOutputStream(std::streambuf& dest) 
: dest_(dest), tag_nest_(0),in_start_tag_(false),xml_document_started_(false)
{
}
FileXMLOutputStream::~FileXMLOutputStream()
{
    // I(tag_nest_ == 0);
    dest_.pubsync();
}

static const std::string xml_head = "<?xml version=\"1.0\" ?>\n";

void FileXMLOutputStream::start_document()
{    
    dest_.sputn(xml_head.c_str(), xml_head.size());
    xml_document_started_ = true;
}
void FileXMLOutputStream::start_tag(const char* tag_name, const char* const* args)
{
    ensure_tag_start_closed();
    maybe_indent();
    dest_.sputc('<');
    dest_.sputn(tag_name,strlen(tag_name));
    in_start_tag_ = true;
    if( args ) {        
        while( *args ) {
            arg(args[0], args[1]);
            args+=2;
        }
    }
}
static void make_indentation(std::streambuf& dest, int level, int indent_size)
{
    for(int i = 0; i < indent_size*level; ++i )
        dest.sputc(' ');
}
void FileXMLOutputStream::arg(const char* name, const char* value)
{
    // I(in_start_tag_);
    dest_.sputc(' ');
    dest_.sputn(name,strlen(name));
    dest_.sputn("=\"",2);
    dest_.sputn(value,strlen(value));
    dest_.sputc('"');
}

void FileXMLOutputStream::char_data(const char* value)
{
    ensure_tag_start_closed();
    maybe_indent();
    dest_.sputn(value,strlen(value));
}

void FileXMLOutputStream::end_tag(const char* tag_name)
{
    if( in_start_tag_ ) {
        dest_.sputn("/>",2);
        in_start_tag_ = false;
    } else {
        --tag_nest_;
        maybe_indent();
        dest_.sputn("</",2);
        dest_.sputn(tag_name, strlen(tag_name));
        dest_.sputc('>');
    }
    maybe_newline();
}

void FileXMLOutputStream::ensure_tag_start_closed()
{
    if( !xml_document_started_ ) {
        start_document();
    }
    if( in_start_tag_ ) {
        dest_.sputc('>');
        maybe_newline();        
        in_start_tag_ = false;
        ++tag_nest_;
    }
    //I(in_start_tag_ == false);
}

void    FileXMLOutputStream::maybe_newline()
{
    if( human_readable_ ) dest_.sputc('\n');
}

void    FileXMLOutputStream::maybe_indent()
{
    if( human_readable_ ) make_indentation(dest_, tag_nest_, 2);
}

///
/// XMLEventBuffer
///

XMLEventBuffer::~XMLEventBuffer()
{
    for( std::vector<XMLEvent*>::iterator i = buffer.begin(); i != buffer.end(); ++i ) {
        delete *i;
    }
    buffer.erase(buffer.begin(), buffer.end());
}

///
/// XMLBufferOutputStream
///

XMLBufferOutputStream::XMLBufferOutputStream(XMLEventBuffer& pevents)
: events(pevents)
{
}


void XMLBufferOutputStream::write(XMLEvent const* ev)
{
    XMLEvent* e = new XMLEvent(*ev);
    events.buffer.push_back(e);
}

void XMLBufferOutputStream::start_tag(const char* tag_name, const char* const* args)
{
    XMLEvent* e = new XMLEvent();
    e->type = XMLEvent::START_TAG;
    e->_tag_name = tag_name;
    e->_args = args_array_copy(args);    
    events.buffer.push_back(e);
}

void XMLBufferOutputStream::arg(const char* name, const char* value)
{
    // TODO:
    // throw tinfra::generic_exception("XMLMemoryStream::arg unimplemented");
}

void XMLBufferOutputStream::end_tag(const char* tag_name)
{
    XMLEvent* e = new XMLEvent();
    e->type = XMLEvent::END_TAG;
    e->_tag_name = tag_name;
    events.buffer.push_back(e);
}

void XMLBufferOutputStream::char_data(const char* value)
{
    XMLEvent* e = new XMLEvent();
    e->type = XMLEvent::CHAR_DATA;
    e->_content = value;
    events.buffer.push_back(e);
}

///
/// XMLBufferInputStream
///

XMLBufferInputStream::XMLBufferInputStream(XMLEventBuffer const& pevents)
: cursor(0), events(pevents)
{
}

XMLEvent* XMLBufferInputStream::peek() {
    if( cursor < events.buffer.size() ) {
        //event_report("PEEK", * events.buffer[cursor], std::cerr);
        return events.buffer[cursor];
    } else {
        //std::cerr << "PEEK: eof" << std::endl;
        return 0;
    }
}

XMLEvent* XMLBufferInputStream::read() {
    if( cursor < events.buffer.size() ) {
        //event_report("READ", * events.buffer[cursor], std::cerr);
        return events.buffer[cursor++];
    } else {
        //std::cerr << "READ: eof" << std::endl;
        return 0; // EOF
    }
}

} } // end of namespace tinfra::xml
