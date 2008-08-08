#include "tinfra/fmt.h"
#include "tinfra/exeinfo.h"

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <iostream>

#include "tinfra/path.h"

// need from system
//    stat
//    path separator, 
//       NO! always use / (only user and incompatible system calls will get system specific paths)
//  
#include <sys/stat.h>

namespace tinfra {
namespace path {

std::string join(const std::string& a, const std::string& b) 
{
    if( a.size() > 0 && b.size() > 0 )
        return a + '/' + b;
    else if( a.size() > 0 ) 
        return a;
    else if (b.size() > 0 )
        return b;
    else
        return "";
}

bool exists(const char* name)
{
    struct stat st;
    return ::stat(name, &st) == 0;
}

static bool is_dir_sep(char a)
{
    return    a == '/' 
           || a == '\\';
}

bool is_dir(const char* name)
{
    int len = strlen(name);
    
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
    std::string tmp;
    if( len > 1 && is_dir_sep(name[len-1]) ) {
        tmp.assign(name, len-1);
        name = tmp.c_str();
    }
#endif
    
    struct stat st;
    if( ::stat(name, &st) != 0 ) 
        return false;
    return (st.st_mode & S_IFDIR) == S_IFDIR;
}

bool is_file(const char* name)
{
    struct stat st;
    if( ::stat(name, &st) != 0 ) 
        return false;
    return (st.st_mode & S_IFREG) == S_IFREG;
}

std::string basename(const std::string& name)
{
    std::string::size_type p = name.find_last_of("/\\");
    if( p == std::string::npos ) {
        return name;
    } else {
        return name.substr(p+1);
    }
}

std::string dirname(const std::string& name)
{
    if( name.size() == 0 ) return ".";
    std::string::size_type p = name.find_last_of("/\\");
    if( p == std::string::npos ) {
        return ".";
    } else if( p == 0 )  {
        return "/";
    } else {
        return name.substr(0,p);
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
