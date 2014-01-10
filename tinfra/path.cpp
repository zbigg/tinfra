// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/platform.h"
#include "config-priv.h"

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

static tstring remove_duplicate_initial_slashes(tstring s)
{
    while( s.size() > 1 && s[0] == '/' && s[1] == '/' ) {
        s = s.substr(1);
    }
    return s;
}

static tstring remove_duplicate_trailing_slashes(tstring s)
{
    size_t size = s.size();
    while( size > 1 && s[size-1] == '/' && s[size-2] == '/' ) {
        s = s.substr(0, size-1);
	size -= 1;
    }
    return s;
}

static tstring remove_duplicate_slashes_at_ends(tstring const& other)
{
    return remove_duplicate_initial_slashes(
               remove_duplicate_trailing_slashes(other));
}
static void calc_join_length(tstring const& s, size_t& result, bool& need_sep_flag)
{
    if( s.size() > 0 ) {
        if( need_sep_flag ) {
            result += 1;
	}
        result += s.size();
        need_sep_flag = true;
    }
}

static void join_append(std::string& result, tstring component, bool& separator_flag)
{
    if( component.size() > 0 ) {
	const bool start_last_is_separator = result.size() > 0 && (result[result.size()-1] == '/');
	const bool first_char_of_component_is_separator = (component[0] == '/');
	if( start_last_is_separator && first_char_of_component_is_separator ) {
	    // if we already have XXX/ and add /FOO then
	    // just advance one char in added component
	    component = component.substr(1);
	} else if( separator_flag && !first_char_of_component_is_separator ) {
	    // if we need separator and it's not already
	    // in place, add it
	    result.append("/");
	}
        result.append(component.data(), component.size());

	const bool result_last_is_separator = (result[result.size()-1] == '/');
	separator_flag = !result_last_is_separator;
    }
}

std::string join(tstring const& p1, tstring const& p2, tstring const& p3, tstring const& p4)
{
    std::string result;

    tstring s1 = remove_duplicate_slashes_at_ends(p1);
    tstring s2 = remove_duplicate_slashes_at_ends(p2);
    tstring s3 = remove_duplicate_slashes_at_ends(p3);
    tstring s4 = remove_duplicate_slashes_at_ends(p4);

    // calculate length
    {
        size_t result_size = 0;
	bool    need_sep_flag = false;
	calc_join_length(s1, result_size, need_sep_flag);
	calc_join_length(s2, result_size, need_sep_flag);
	calc_join_length(s3, result_size, need_sep_flag);
	calc_join_length(s4, result_size, need_sep_flag);
	result.reserve(result_size+1);
    }
    {
        bool separator_flag = false;
        join_append(result, s1, separator_flag);
        join_append(result, s2, separator_flag);
        join_append(result, s3, separator_flag);
        join_append(result, s4, separator_flag);
    }
    return result;
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

std::string sanitize(tstring const& path)
{
    
    return path;
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
        sprefix = remove_extension(basename(get_exepath()));
    } else {
        sprefix = prefix;
    }
    const std::string result = fmt("%s/%s_%s_%s") % tmpdir % sprefix % t % stamp;
    return result; 
}

#ifdef _WIN32
static std::vector<std::string> get_executable_extensions() 
{
    const char* pathext = std::getenv("PATHEXT");
    if( pathext == 0 )
        pathext = ".com;.exe;.bat;.cmd";
    return split(pathext, ';');
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


std::string extension(tstring const& filename)
{
    
    size_t last_dot = filename.find_last_of(".");
    if( last_dot == tstring::npos )
        return "";
    
    size_t last_slash = filename.find_last_of("\\/");
    if( last_slash != tstring::npos && last_slash > last_dot )
        // extension is somewhere in path component
        return "";
        
    return filename.substr(last_dot+1).str();
    
}

std::string remove_extension(tstring const& filename)
{
	const size_t last_slash = filename.find_last_of("\\/");
	const size_t last_dot = filename.find_last_of('.');
	if( ( last_slash != tstring::npos && last_dot < last_slash) || 
	    ( last_dot == tstring::npos) ||
		( last_dot == filename.size() - 1 )
		) 
	{
		return filename.str();
	} else {
		tstring result = filename.substr(0, last_dot);
		return result.str();
	}
}

std::string remove_all_extensions(tstring const& filename)
{
	const size_t last_slash = filename.find_last_of("\\/");
	const size_t last_dot = filename.find_first_of('.', last_slash);
	if( ( last_dot == tstring::npos ) ||
		( last_dot == filename.size() - 1 ) ) 
	{
		return filename.str();
	} else {
		tstring result = filename.substr(0, last_dot);
		return result.str();
	}
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
static const char PATH_SEPARATOR = ';';
#else
static const char PATH_SEPARATOR = ':';
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

