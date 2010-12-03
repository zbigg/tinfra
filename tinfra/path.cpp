//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/platform.h"

#include "tinfra/path.h"
#include "tinfra/fs.h"

#include "tinfra/fmt.h"
#include "tinfra/exeinfo.h"
#include "tinfra/string.h"

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#include <cstdlib>
#include <cctype>
#include <string.h>



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
    time(&t);
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

#ifdef _WIN32
static std::vector<std::string> get_executable_extensions() 
{
    const char* pathext = std::getenv("PATHEXT");
    if( pathext == 0 )
        pathext = ".com;.exe;.bat;.cmd";
    return split(pathext, ";");
}

bool is_executable(tstring const& name, std::vector<std::string> const& extensions)
{
    // check existence
    if( !tinfra::fs::exists(name) )
        return false;
    
    // check correct extension
    const size_t name_len = name.size();
    for( std::vector<std::string>::const_iterator iext = extensions.begin();
          iext != extensions.end(); ++iext)
    {
        const std::string& ext    = *iext;
        const size_t       extlen = ext.size();
        
        if( name_len < extlen )
            continue;
        
        const tstring actual_ext = name.substr(name_len-extlen, extlen);
        if( compare_no_case(actual_ext.data(), ext.data(), extlen) == 0 ) {
            return true;
        }
    }
    return false;
}

#endif

bool is_executable(tstring const& name)
{
#ifdef _WIN32
    // check PATHEXT or ".exe .com .bat .cmd"
    std::vector<std::string> extensions = get_executable_extensions();
    return is_executable(name, extensions);
#else
    string_pool temporary_context;
    struct stat st;
    if( ::stat(name.c_str(temporary_context), &st) != 0 ) 
        return false;
    const bool regular_file = (st.st_mode & S_IFREG) == S_IFREG; 
    const bool executable   = (st.st_mode & 0111) != 0;
    return regular_file && executable;
#endif
}

bool is_absolute(tstring const& filename)
{
    if( filename.size() == 0 )
        return false;
    if( filename[0] == '/' || filename [0] == '\\')
        return true;
#ifdef _WIN32
    if( filename.size() >= 2 && std::isalpha(filename[0]) && filename[1] == ':' )
        return true;
#endif
    return false;
}

bool has_extension(tstring const& filename)
{
    size_t find_start = filename.find_last_of("\\/");
    if( find_start == tstring::npos )
        find_start = 0;
    else
        find_start++;
    
    const size_t dot_pos = filename.find_first_of('.', find_start);
    const bool dot_exists =   (dot_pos != tstring::npos);
    const bool not_at_start = (dot_pos > find_start);
    return dot_exists && not_at_start;
}

#ifdef _WIN32
static const char* PATH_SEPARATOR = ";";
#else
static const char* PATH_SEPARATOR = ":";
#endif

#ifdef _WIN32

///
/// Check if file with appended various extensions
/// exists.
///
/// 
/// @return empty if none of variant exists or name of firts variant that exists

static std::string find_variant(tstring const& filename, std::vector<std::string> const& extensions)
{
    std::string pathext;
    for( std::vector<std::string>::const_iterator iext = extensions.begin(); iext != extensions.end(); ++iext ) {
        pathext.reserve(filename.size() + iext->size());
        
        pathext.assign(filename.data(), filename.size());
        pathext.append(*iext);
        
        if( fs::exists(pathext) ) {
            return pathext;
        }
    }
    return "";
}

std::string search_executable(tstring const& filename, tstring const& path)
{
    const bool filename_has_extension    = has_extension(filename);
    const bool filename_is_absolute_path = is_absolute(filename);
    
    const std::vector<std::string> extensions = get_executable_extensions();  
    
    if( ! filename_is_absolute_path ) {
        const std::vector<std::string> dirs = split(path, PATH_SEPARATOR);
        
        for(std::vector<std::string>::const_iterator ipath = dirs.begin(); ipath != dirs.end(); ++ipath ) {
            const std::string path1 = tinfra::path::join(*ipath, filename);
            if( filename_has_extension ) {
                if( is_executable(path1 ,extensions) )
                    return path1;
                continue;
            }
            std::string maybe_with_ext = find_variant(path1, extensions);
            if( ! maybe_with_ext.empty() )
                return maybe_with_ext;
        }
    } else if( ! filename_has_extension ) {
        return find_variant(filename.str(), extensions);
    } else {
        if( is_executable(filename, extensions) ) {
            return filename.str();
        }
    }
    return "";
}

// end of win32 version of search_executable
#else 
// start of common, posix version

// TODO: move it to posix_common.sh
std::string search_executable(tstring const& filename, tstring const& path)
{
    if( is_absolute(filename) )
        if( is_executable(filename) )
            return filename.str();
    
    std::vector<std::string> dirs = split(path, PATH_SEPARATOR);
    
    for(std::vector<std::string>::const_iterator ipath = dirs.begin(); ipath != dirs.end(); ++ipath )
    {
        std::string result_name = tinfra::path::join(*ipath, filename);
        if( is_executable(result_name) ) {
            return result_name;
        }
    }
    return "";
}
#endif

std::string search_executable(tstring const& filename)
{
    const char* path = std::getenv("PATH");
    if( path == 0 )
        path = "";
    return search_executable(filename, path);
}

} } // end namespace tinfra::path

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

