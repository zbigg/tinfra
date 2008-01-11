#ifndef __tinfra_exeinfo_h__
#define __tinfra_exeinfo_h__

#include <istream>

namespace tinfra {

struct symbol_info {
    intptr_t    address;
    char const* name;
    char const* file_name;
    int         line_number;
};

typedef void (*symbol_table_visitor)(symbol_info const&);

void read_symbol_map(std::istream& input, symbol_table_visitor visitor);

std::string get_exepath();
void set_exepath(std::string const& path);

}

#endif
