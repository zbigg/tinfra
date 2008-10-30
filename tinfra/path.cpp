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

static bool is_dir_sep(char a)
{
    return    a == '/' 
           || a == '\\';
}

bool is_dir(tstring const& name)
{
    size_t len = name.size();
    
    if( len == 1 && name[0] == '.' )      // current directory
        return true;
    
    if( len == 1 && is_dir_sep(name[0]) ) // single backslash 
        return true;
    
#ifdef _WIN32
    if( len >= 2 && std::isalpha(name[0]) && name[1] == ':' ) {
        if( len == 2 )
            return true; // A:
        if( len == 3 && is_dir_sep(name[2]) )
            return true; // A:\ and A:/
    }
    // NOTE: win32 stat doesn't accept trailing slash/back 
    // slash in folder name
    if( len > 1 && is_dir_sep(name[len-1]) ) {
        tstring tmp(name.data(), len-1);
        return is_dir(name);
    }
#endif
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

std::string tmppath(const char* prefix, const char* tmpdir)
{
    if( tmpdir == 0 || strlen(tmpdir) == 0 ) {
        tmpdir  = ::getenv("TMP");
        if( !tmpdir) 
            tmpdir = ::getenv("TEMP");
        #ifdef _WIN32
            if( !tmpdir) 
                tmpdir = "/Temp";
        #else
            if( !tmpdir) 
                tmpdir = "/tmp";
        #endif
    }       
    // TODO: it's somewhat weak radnomization strategy
    //       invent something better
    time_t t;
    ::time(&t);
    static bool srand_called = false;
    if( !srand_called ) {
        srand_called = true;
        ::srand(static_cast<unsigned>(t));
    }
    int stamp = ::rand() % 104729; // 104729 is some arbitrary prime number
    
    std::string sprefix;
    if( prefix == 0 || strlen(prefix) == 0) {
        sprefix = basename(get_exepath()).c_str();
    } else {
        sprefix = prefix;
    }
    return fmt("%s/%s_%s_%s") % tmpdir % sprefix % t % stamp;
}

} }
