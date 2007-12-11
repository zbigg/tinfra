#include "tinfra/xml/XMLStream.h"

namespace tinfra { 
namespace xml {
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
: dest_(dest), tag_nest_(0),in_start_tag_(false)
{
}
FileXMLOutputStream::~FileXMLOutputStream()
{
    // I(tag_nest_ == 0);
    dest_.pubsync();
}
void FileXMLOutputStream::start_tag(const char* tag_name, const char* const* args)
{
    ensure_tag_start_closed();
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
    dest_.sputn(value,strlen(value));
}
void FileXMLOutputStream::end_tag(const char* tag_name)
{
    if( in_start_tag_ ) {
        dest_.sputn("/>",2);
        in_start_tag_ = false;
    } else {
        dest_.sputn("</",2);
        dest_.sputn(tag_name, strlen(tag_name));
        dest_.sputc('>');
        --tag_nest_;
    }
}

void FileXMLOutputStream::ensure_tag_start_closed()
{
    if( in_start_tag_ ) {
        dest_.sputc('>');
        in_start_tag_ = false;
        ++tag_nest_;
    }
    //I(in_start_tag_ == false);
}

///
/// XMLMemoryStream
///

XMLMemoryStream::XMLMemoryStream()
: read_cursor(0), write_cursor(0)
{
}

XMLMemoryStream::~XMLMemoryStream()
{
    for( std::vector<XMLEvent*>::iterator i = events.begin(); i != events.end(); ++i ) {
        delete *i;
    }
    events.erase(events.begin(), events.end());
}

void XMLMemoryStream::write(XMLEvent const* ev)
{
    XMLEvent* e = new XMLEvent(*ev);
    events.push_back(e);
}

void XMLMemoryStream::start_tag(const char* tag_name, const char* const* args)
{
    XMLEvent* e = new XMLEvent();
    e->type = XMLEvent::START_TAG;
    e->_tag_name = tag_name;
    int n = 0;
    {
        const char* const* ia = args;
        while( *ia ) {
            ia += 2;
            n++;
        }
    }
    //cout << "TAG " << name << "(";
    {
        e->_args = new char*[n*2+1];
        char** idest = e->_args;
        const char* const* isrc  = args;
        while( *isrc ) {
            idest[0] = strdup(isrc[0]);
            idest[1] = strdup(isrc[1]);
            //cout << idest[0] << "=" << idest[1] << " ";
            idest += 2;
            isrc  += 2;
        }
        idest[0] = 0;
    }
    //cout << ")" << endl;
    events.push_back(e);
}

void XMLMemoryStream::arg(const char* name, const char* value)
{
    // TODO:
    // throw tinfra::generic_exception("XMLMemoryStream::arg unimplemented");
}

void XMLMemoryStream::end_tag(const char* tag_name)
{
    XMLEvent* e = new XMLEvent();
    e->type = XMLEvent::END_TAG;
    e->_tag_name = tag_name;
    events.push_back(e);
}

void XMLMemoryStream::char_data(const char* value)
{
    XMLEvent* e = new XMLEvent();
    e->type = XMLEvent::CHAR_DATA;
    e->_content = value;
    events.push_back(e);
}

XMLEvent* XMLMemoryStream::peek() {
    if( read_cursor < events.size() ) {
        return events[read_cursor];
    } else {
        return 0;
    }
}

XMLEvent* XMLMemoryStream::read() {
    if( read_cursor < events.size() ) {
        //cerr << "READ: "; events[cursor]->report();
        return events[read_cursor++];
    } else {
        return 0; // EOF
    }
}

} } // end of namespace tinfra::xml
