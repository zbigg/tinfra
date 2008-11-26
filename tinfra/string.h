//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_string_h__
#define __tinfra_string_h__

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
