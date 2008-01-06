
#include "tinfra/fs.h"
#include "tinfra/path.h"
#include "tinfra/fmt.h"
#include <streambuf>
#include <fstream>
#include <ios>

#include <sys/types.h>
#include <sys/stat.h>

#include <string.h>
#include <errno.h>
#include <iostream>

#if defined _WIN32
#include <direct.h>
#endif

#define HAVE_OPENDIR

#ifdef HAVE_OPENDIR
#include <dirent.h>
#endif

namespace tinfra {
namespace fs {

template <typename T>
struct holder {
    T value_;
    holder(T const& value): value_(value) {}
    ~holder() { dispose(value_); }
    
    static void dispose(T value);
};

#ifdef HAVE_OPENDIR
void holder<DIR*>::dispose(DIR* dir)
{
    ::closedir(dir);
    dir = 0;
}
#endif

void list_files(const char* dirname, std::vector<std::string>& result)
{
#ifdef HAVE_OPENDIR
    DIR* dir = ::opendir(dirname);
    if( !dir ) {
        std::string error_str = ::strerror(errno);
        throw generic_exception(fmt("unable to read dir '%s': %s") % dirname % error_str);
    }
    holder<DIR*> dir_closer(dir);
    dirent* entry;
    while( (entry = ::readdir(dir)) != 0 ) 
    {
        std::string name(entry->d_name);
        if( name == ".." || name == "." ) continue;
        result.push_back(name);
    }    
#else
    throw generic_exception("tinfra::fs::list_files not implemented on this platform");
#endif    
}

void recursive_copy(const char* src, const char* dest)
{
    if( path::is_dir(dest) ) {
        std::string new_dest = path::join(dest, path::basename(src));
        return recursive_copy(src, new_dest.c_str());
    } else if( path::is_dir(src) ) {
        mkdir(dest);
        std::vector<std::string> files;
        list_files(src, files);
        for( std::vector<std::string>::const_iterator i = files.begin(); i!=files.end(); ++i )
        {
            std::string new_src = path::join(src, *i);
            std::string new_dest = path::join(dest, *i);
            recursive_copy(new_src, new_dest);
        }
    } else {
        copy(src, dest);
    }
}

void recursive_rm(const char* name)
{
    if( path::is_dir(name) ) {
        std::vector<std::string> files;
        list_files(name, files);
        
        for( std::vector<std::string>::const_iterator i = files.begin(); i!=files.end(); ++i ) 
        {
            recursive_rm( path::join(name, *i).c_str() );
        }
        rmdir(name);
    } else {        
        rm(name);
    }
}

void rm(const char* name)
{
    int result = ::unlink(name);
    if( result == -1 ) {
        std::string error_str = ::strerror(errno);
        throw generic_exception(fmt("unable to remove '%s': %s") % name % error_str);
    }
}

void rmdir(const char* name)
{
    int result = ::rmdir(name);
    if( result == -1 ) {
        std::string error_str = ::strerror(errno);
        throw generic_exception(fmt("unable to remove dir '%s': %s") % name % error_str);
    }
}

void mkdir(const char* name, bool create_parents)
{
    std::string parent = path::dirname(name);
    if( !path::is_dir(parent) )
        if( create_parents )
            mkdir(parent.c_str());
        else
            throw generic_exception(fmt("unable to create dir '%s': %s") % name % "parent folder doesn't exist");
#ifdef _WIN32
    int result = ::mkdir(name);
#else
    int result = ::mkdir(name, 0777);
#endif
    if( result == -1 ) {
        std::string error_str = strerror(errno);
        throw generic_exception(fmt("unable to create dir '%s': %s") % name % error_str);
    }
}

static void copy(std::streambuf& in, std::streambuf& out)
{
    char buffer[8192];
    std::streamsize readed;
    while( (readed = in.sgetn(buffer, sizeof(buffer))) > 0 ) 
    {
        std::streamsize written = 0;
        while( written < readed ) 
        {
            std::streamsize wt = out.sputn(buffer + written, readed-written);
            if( wt < 0 ) {
                std::string error_str = "?";
                throw generic_exception(fmt("error writing file: %s") % error_str);
            }
            written += wt;
        }
    }
}

void copy(const char* src, const char* dest)
{
    if( path::is_dir(src) ) 
        throw generic_exception("tinfra::fs::copy supports only generic files");
    if( path::is_dir(dest) ) {
        copy(src, path::join(dest, path::basename(src) ).c_str());
        return;
    }
    std::filebuf in;
    if( !in.open(src, std::ios::in | std::ios::binary) ) {
        std::string error_str = "?";
        throw generic_exception(fmt("unable to open input '%s': %s") % src % error_str);
    }
    
    std::filebuf out;
    if( ! out.open(dest, std::ios::out | std::ios::trunc | std::ios::binary) ) 
    {
        std::string error_str = "?";
        throw generic_exception(fmt("unable to open output '%s': %s") % dest % error_str);
    }
    
    copy(in, out);
}

void cd(const char* dirname)
{
    int result = ::chdir(dirname);
    if( result == -1 ) {
        std::string error_str = ::strerror(errno);
        throw generic_exception(fmt("unable to open output '%s': %s") % dirname % error_str);
    }
}

std::string pwd()
{
    char buf[1024];
    if( getcwd(buf, sizeof(buf)) == 0 ) {
        throw generic_exception("fs::pwd unable read pwd (implement it better!)");
    }
    return std::string(buf);
}

} }

