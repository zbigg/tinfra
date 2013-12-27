//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/platform.h"

#include "tinfra/exeinfo.h"
#include "tinfra/fs.h"

#include <istream>
#include <cstdio>
#include <cstring>
#include <cctype>

namespace tinfra {

static std::string exepath = "";

std::string get_exepath()
{
    if( exepath.size() == 0 ) {
        // TODO: write some OS-dependent code here
#ifdef linux
        if( tinfra::fs::exists("/proc/self/exe") ) {
            return tinfra::fs::readlink("/proc/self/exe");
        } 
#endif   
		return "";
    } else {
        return exepath;
    }
}
void set_exepath(std::string const& path)
{    
    exepath = path;
}

} // end namespace tinfra
