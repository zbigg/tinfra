#include "inifile.h"

namespace tinfra {
namespace inifile {

struct parser::internal_data {
    std::istream& input;
    
    internal_data(std::istream& in): 
        input(in)
    {}
};

parser::parser(std::istream& in):
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

bool parser::fetch_next(entry& out)
{
    std::string line;
    bool result = getline(data_->input, line);
    if( !result )
        return false;
    
    if( line.size() == 0 ) {
        out.type = EMPTY;
        out.value = "";
        return true;
    }
    if( line[0] == ';' ) {
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
    
    const size_t entry_division_start = line.find_first_of(" \t=");
    if( entry_division_start == std::string::npos ) {
        fill_invalid_entry(out, "invalid line, not a section, entry or comment", line);
        return true;
    }
    
    out.type = ENTRY;
    out.name = line.substr(0, entry_division_start);
    const size_t value_begin = line.find_first_not_of(" \t=", entry_division_start);
    // TODO: escape sequence support
    // TODO: unicode support
    // TODO: inline comment support
    const size_t value_end = line.find_last_not_of(" \t");
    if( value_end != std::string::npos ) 
        out.value = line.substr(value_begin, 1+value_end-value_begin);
    else
        out.value = line.substr(value_begin);
    return true;
}

} } // end namespace tinfra::inifile




