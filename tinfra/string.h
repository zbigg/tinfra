//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_string_h_included
#define tinfra_string_h_included

#include <string>
#include <vector>
#include "tinfra/tstring.h"

namespace tinfra {

    
void        escape_c_inplace(std::string& a);
std::string escape_c(tstring const& a);
    
void        strip_inplace(std::string& s);
std::string strip(tstring const& s);

void        chop_inplace(std::string& s);
std::string chop(tstring const& s);

std::vector<std::string> split(tstring const& in, tstring const& delimiters);
std::vector<std::string> split_lines(tstring const& in);
    
}

#endif
