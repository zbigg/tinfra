//
// Copyright (c) 2009-2011, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "inifile.h"
#include "trace.h"
#include "string.h" // for strip
#include "tinfra/fmt.h"    // for fmt

namespace tinfra {
namespace inifile {

tinfra::module_tracer inifile_tracer(tinfra::tinfra_tracer, "inifile");

using tinfra::fmt;
    
struct parser::internal_data {
    tinfra::input_stream& input;
    
    internal_data(tinfra::input_stream& in): 
        input(in)
    {}
};

parser::parser(tinfra::input_stream& in):
    data_(new internal_data(in))
{
}

parser::~parser()
{
}

void fill_invalid_entry(entry& out, std::string const& error, std::string const& value)
{
    out.type = INVALID;
    out.name = error;
    out.value = value;
}

static bool readline(tinfra::input_stream& in, std::string& result)
{
    int readed = 0;
    while( true ) {
        char c;
        const int r = in.read(&c, 1);
        if( r == 0 ) {
            break;
        }

        readed += 1;
        if( c== '\n' )
            break;
        result.append(1, c);
    }
    return readed != 0;
}

bool parser::fetch_next(entry& out)
{
    std::string line;
    const bool have_result = readline(data_->input, line);
    if( !have_result )
        return false;
    
    if( line.size() == 0 ) {
        out.type = EMPTY;
        out.value = "";
        return true;
    }
    if( line[0] == ';' || line[0] == '#' ) {
        out.type = COMMENT;
        out.value = line.substr(1);
        return true;
    }
    if( line[0] == '[' ) {
        const size_t end_bracked_pos = line.find(']');
        if( end_bracked_pos == std::string::npos ) {
            fill_invalid_entry(out, "no ] at end of section", line);
            return true;
        }
        out.type = SECTION;
        out.name = line.substr(1, end_bracked_pos-1);
        out.value = line;
        return true;
    }
    
    const size_t entry_division_start = line.find_first_of("=");
    if( entry_division_start == std::string::npos ) {
        fill_invalid_entry(out, "invalid line, not a section, entry or comment", line);
        return true;
    }
    
    out.type = ENTRY;
    out.name = tinfra::strip(line.substr(0, entry_division_start));
    const size_t value_begin = line.find_first_not_of(" \t=", entry_division_start);
    if( value_begin == std::string::npos ) {
        out.value = "";
        return true;
    }
    // TODO: escape sequence support
    // TODO: unicode support
    // TODO: inline comment support
    const size_t value_end = line.find_last_not_of(" \r\n\t");
    if( value_end != std::string::npos ) 
        out.value = line.substr(value_begin, 1+value_end-value_begin);
    else
        out.value = line.substr(value_begin);
    return true;
}

reader::reader(tinfra::input_stream& in):
    p(in),
    line(0)
{
}
reader::~reader()
{
}

bool reader::fetch_next(full_entry& result)
{
	
	while( true ) {
		entry e;
		if( !p.fetch_next(e) )
			return false;
		this->line++;
		switch( e.type ) {
		case EMPTY:
		case COMMENT:
			break;
		case INVALID:
			TINFRA_TRACE(inifile_tracer, "invalid inifile entry, line=" << line << " content=" << e.value);
			break;
		case SECTION:
			this->section = e.name;
			break;
		case ENTRY:
			result.section = this->section;
			result.name = e.name;
			result.value = e.value;
			return true;
		}
	}
}

//
// tinfra::inifile::writer
//

writer::writer(tinfra::output_stream& o): out(o) {}
writer::~writer() {}

void writer::section(std::string const& name)
{
    std::string const formatted = (fmt("[%s]\n") % name).str();
    this->out.write(formatted);
}

void writer::entry(std::string const& name, std::string const& value)
{
    std::string const formatted = (fmt("%s = %s\n") % name % value).str();
    this->out.write(formatted);
}

void writer::comment(std::string const& content)
{
    std::string const formatted = (fmt("; %s\n") % content).str();
    this->out.write(formatted);
}

void writer::entry(tinfra::inifile::entry const& e)
{
    switch( e.type ) {
    case SECTION: section(e.name); break;
    case ENTRY:   entry(e.name, e.value); break;
    case COMMENT: comment(e.value); break;
    case EMPTY:   break;
    case INVALID: break; // shouldn't we just output it ???
    }
}

} } // end namespace tinfra::inifile

