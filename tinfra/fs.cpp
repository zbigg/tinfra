
#include "tinfra/platform.h"
#include "tinfra/fs.h"
#include "tinfra/path.h"
#include "tinfra/fmt.h"
#include "tinfra/io/stream.h"
#include <streambuf>
#include <fstream>
#include <ios>

#include <sys/types.h>
#include <sys/stat.h>

#include <cstring>
#include <errno.h>
#include <iostream>

#if defined _WIN32
#include <direct.h>
#endif

#ifdef HAVE_IO_H
#include <io.h>
#endif

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
template<>
void holder<DIR*>::dispose(DIR* dir)
{
    ::closedir(dir);
    dir = 0;
}
#elif defined HAVE_FINDFIRST
template<>
void holder<intptr_t>::dispose(intptr_t i)
{
    _findclose(i);
}
#endif

void list_files(const char* dirname, file_list_visitor& visitor)
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
        if( std::strcmp(entry->d_name,"..") == 0 || std::strcmp(entry->d_name,".") == 0 ) continue;
        visitor.accept(entry->d_name);
    }    
#elif defined HAVE_FINDFIRST
	std::string a = dirname;
	a += "\\*";
	_finddata_t finddata;
	intptr_t nonce = _findfirst(a.c_str(), &finddata);
	if( nonce == -1 ) 
		return;
        holder<intptr_t> nonce_closer(nonce);
	do {
		if( std::strcmp(finddata.name,"..") != 0 && std::strcmp(finddata.name, ".") != 0 ) {
			visitor.accept(finddata.name);
		}
	} while( _findnext(nonce, &finddata) == 0);	
#else
    throw generic_exception("tinfra::fs::list_files not implemented on this platform");
#endif  
}

struct vector_sink_file_visitor: public file_list_visitor {
    std::vector<std::string>& result;
    vector_sink_file_visitor(std::vector<std::string>& result): result(result) {}
        
    virtual void accept(const char* name)
    {
        result.push_back(name);
    }
};

void list_files(const char* dirname, std::vector<std::string>& result)
{
    vector_sink_file_visitor visitor(result);
    list_files(dirname, visitor);
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
    
    tinfra::io::copy(in, out);
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

namespace {
static void walk_(const char* start, walker& walker);
struct walker_file_visitor: public tinfra::fs::file_list_visitor {
    const char* parent_;
    walker& walker_;
    
    walker_file_visitor(const char* parent, walker& walker): parent_(parent), walker_(walker) {}
        
    void accept(const char* name)
    {
        std::string file_path(tinfra::path::join(parent_, name));
        bool is_dir = tinfra::path::is_dir(file_path);
        bool dig_further = walker_.accept(name,  parent_, is_dir);
        if( is_dir && dig_further ) {
            walk_(file_path.c_str(), walker_);
        }
    }
};

static void walk_(const char* start, walker& w)
{    
    walker_file_visitor visitor(start, w);
    tinfra::fs::list_files(start, visitor);    
}
}

void walk(const char* start, walker& w)
{
    try 
    {
        walk_(start, w);
    } 
    catch(walker::stop) { }
}

} }

