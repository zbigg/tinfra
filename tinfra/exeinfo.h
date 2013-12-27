//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_exeinfo_h_included
#define tinfra_exeinfo_h_included

#include "platform.h" // for intptr_t

#include <istream>

namespace tinfra {

std::string get_exepath();
void set_exepath(std::string const& path);

}

#endif
