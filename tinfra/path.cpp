#include "tinfra/path.h"

// need from system
//    stat
//    path separator, 
//       NO! always use / (only user and incompatible system calls will get system specific paths)
//  
#include <sys/stat.h>

namespace tinfra {
namespace path {

std::string join(const std::string& a, const std::string b) 
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

bool is_dir(const char* name)
{
    struct stat st;
    return ::stat(name, &st) == 0 && (st.st_mode & S_IFDIR) == S_IFDIR;
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

} }
