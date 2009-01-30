//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/fmt.h"
#include "tinfra/exeinfo.h"

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#include <cstdlib>

#include "tinfra/path.h"

// need from system
//    stat
//    path separator, 
//       NO! always use / (only user and incompatible system calls will get system specific paths)
//  
#include <sys/stat.h>

namespace tinfra {
namespace path {

std::string join(tstring const& a, tstring const& b) 
{
    if( a.size() > 0 && b.size() > 0 ) {
        std::string r;
        r.reserve(a.size() + 1 + b.size());
        r.append(a.data(), a.size());
        r.append(1, '/');
        r.append(b.data(), b.size());
        return r;
    }
    else if( a.size() > 0 ) 
        return a.str();
    else if (b.size() > 0 )
        return b.str();
    else
        return "";
}

bool exists(tstring const& name)
{
    string_pool temporary_context;
    struct stat st;
    return ::stat(name.c_str(temporary_context), &st) == 0;
}

bool is_dir(tstring const& name)
{
    string_pool temporary_context;
    struct stat st;
    if( ::stat(name.c_str(temporary_context), &st) != 0 ) 
        return false;
    return (st.st_mode & S_IFDIR) == S_IFDIR;
}

bool is_file(tstring const& name)
{
    string_pool temporary_context;
    struct stat st;
    if( ::stat(name.c_str(temporary_context), &st) != 0 ) 
        return false;
    return (st.st_mode & S_IFREG) == S_IFREG;
}

std::string basename(tstring const& name)
{
    std::string::size_type p = name.find_last_of("/\\");
    if( p == tstring::npos ) {
        return name.str();
    } else {
        return std::string(name.data()+p+1, name.size()-p-1);
    }
}

std::string dirname(tstring const& name)
{
    if( name.size() == 0 ) return ".";
    tstring::size_type p = name.find_last_of("/\\");
    if( p == tstring::npos ) {
        return ".";
    } else if( p == 0 )  {
        return "/";
    } else {
        return std::string(name.data(), p);
    }
}

std::string tmppath()
{
    std::string result;    
    const char* tmpdir  = std::getenv("TMP");
    if( !tmpdir) tmpdir = std::getenv("TEMP");
#ifdef _WIN32
    if( !tmpdir) tmpdir = "/Temp";
#else
    if( !tmpdir) tmpdir = "/tmp";
#endif
    // TODO: it's somewhat weak radnomization strategy
    //       invent something better
    time_t t;
    time(&t);
    static bool srand_called = false;
    if( !srand_called ) {
        srand_called = true;
        ::srand(static_cast<unsigned>(t));
    }
    int stamp = ::rand() % 104729; // 104729 is some arbitrary prime number
    
    return fmt("%s/%s_%s_%s") % tmpdir % basename(get_exepath()) % t % stamp;
}

} }
