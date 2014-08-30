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
#include <cstdlib>

#if __APPLE__ && __MACH__
#include <mach-o/dyld.h>
#endif

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
#if __APPLE__ && __MACH__
        const uint32_t  start_size = 4;
        uint32_t size = start_size;
        {
	    char path[start_size];
            if (_NSGetExecutablePath(path, &size) == 0)
                return std::string(path);
        }
        char* path = reinterpret_cast<char*>(std::malloc(size));
        std::string result;
        if (_NSGetExecutablePath(path, &size) == 0) {
            // TODO: yeah, we std::string const can throw here
            // but who cares what happens in case of catastrophe like OOM
            result = path;
        }
        std::free(path);
        return result;
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
