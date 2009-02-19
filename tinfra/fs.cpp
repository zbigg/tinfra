//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//


#include "tinfra/platform.h"
#include "tinfra/fs.h"
#include "tinfra/path.h"
#include "tinfra/fmt.h"
#include "tinfra/io/stream.h"
#include "tinfra/os_common.h"
#include "tinfra/vfs.h"
#include <streambuf>
#include <fstream>
#include <stdexcept>
#include <ios>
#include <memory>

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

void list_files(tstring const& dirname, file_list_visitor& visitor)
{
    string_pool temporary_context;
#ifdef HAVE_OPENDIR
    DIR* dir = ::opendir(dirname.c_str(temporary_context));
    if( !dir ) {
        throw_errno_error(errno, fmt("unable to read dir '%s'") % dirname);
    }
    holder<DIR*> dir_closer(dir);
    dirent* entry;
    while( (entry = ::readdir(dir)) != 0 ) 
    {        
        if( std::strcmp(entry->d_name,"..") == 0 || std::strcmp(entry->d_name,".") == 0 ) continue;
        visitor.accept(entry->d_name);
    }    
#elif defined HAVE_FINDFIRST
    std::string a = dirname.str();
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
        
    virtual void accept(tstring const& name)
    {
        result.push_back(name.str());
    }
};

void list_files(tstring const& dirname, std::vector<std::string>& result)
{
    vector_sink_file_visitor visitor(result);
    list_files(dirname, visitor);
}

file_info stat(tstring const& name)
{
    string_pool temporary_context;
    struct stat st;
    if( ::stat(name.c_str(temporary_context), &st) != 0 ) {
        throw_errno_error(errno, fmt("unable stat file '%s'") % name);
    }
    
    file_info result;
    result.is_dir = (st.st_mode & S_IFDIR) == S_IFDIR;
    result.modification_time = st.st_mtime;
    result.access_time = st.st_atime;
    result.size = st.st_size;
    return result;
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
        return is_dir(tmp);
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

void mv(tstring const& src, tstring const& dest)
{
    string_pool tmp_pool;
    int result = ::rename(src.c_str(tmp_pool), dest.c_str(tmp_pool));
    if( result == -1 ) {
        throw_errno_error(errno, fmt("unable to rename from '%s' to '%s' ") % src % dest);
    }    
}

void rm(tstring const& name)
{
    string_pool temporary_context;
    int result = ::unlink(name.c_str(temporary_context));
    if( result == -1 ) {
        throw_errno_error(errno, fmt("unable to remove file '%s'") % name);
    }
}

void rmdir(tstring const& name)
{
    string_pool temporary_context;
    int result = ::rmdir(name.c_str(temporary_context));
    if( result == -1 ) {
        throw_errno_error(errno, fmt("unable to remove folder '%s'") % name);
    }
}

void mkdir(tstring const& name, bool create_parents)
{
    std::string parent = path::dirname(name);
    if( !is_dir(parent) ) {
        if( create_parents )
            mkdir(parent);
        else
            throw std::logic_error(fmt("unable to create dir '%s': %s") % name % "parent folder doesn't exist");
    }
    string_pool temporary_context;
#ifdef _WIN32
    int result = ::mkdir(name.c_str(temporary_context));
#else
    int result = ::mkdir(name.c_str(temporary_context), 0777);
#endif
    if( result == -1 ) {
        throw_errno_error(errno, fmt("unable to create dir '%s'") % name);
    }
}

void recursive_copy(tstring const& src, tstring const& dest)
{
    tinfra::vfs& fs = tinfra::local_fs();
    tinfra::default_recursive_copy(fs, src, fs, dest);
}

void recursive_rm(tstring const& name)
{
    tinfra::vfs& fs = tinfra::local_fs();
    return tinfra::default_recursive_rm(fs, name);
}

void copy(tstring const& src, tstring const& dest)
{
    tinfra::vfs& fs = tinfra::local_fs();
    return tinfra::default_copy(fs, src, fs, dest);
}

void cd(tstring const& dirname)
{
    string_pool temporary_context;
    int result = ::chdir(dirname.c_str(temporary_context));
    if( result == -1 ) {
        throw_errno_error(errno, fmt("unable to open output '%s'") % dirname);
    }
}

std::string pwd()
{
    char buf[1024];
    if( getcwd(buf, sizeof(buf)) == 0 ) {
        throw_errno_error(errno, "fs::pwd() unable read pwd (implement it better!)");
    }
    return std::string(buf);
}

namespace {
    
static void walk_(tstring const& start, walker& walker);
    
struct walker_file_visitor: public tinfra::fs::file_list_visitor {
    tstring const& parent_;
    walker& walker_;
    
    walker_file_visitor(tstring const& parent, walker& walker): parent_(parent), walker_(walker) {}
        
    void accept(tstring const& name)
    {
        std::string file_path(tinfra::path::join(parent_, name));
        bool dir = is_dir(file_path);
        bool dig_further = walker_.accept(name,  parent_, dir);
        if( dir && dig_further ) {
            walk_(file_path, walker_);
        }
    }
};

static void walk_(tstring const& start, walker& w)
{    
    walker_file_visitor visitor(start, w);
    tinfra::fs::list_files(start, visitor);    
}
}

void walk(tstring const& start, walker& w)
{
    try 
    {
        walk_(start, w);
    } 
    catch(walker::stop) { }
}

} }

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

