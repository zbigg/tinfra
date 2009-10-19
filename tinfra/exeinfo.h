//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_exeinfo_h_included
#define tinfra_exeinfo_h_included

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
