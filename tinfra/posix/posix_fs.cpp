//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/platform.h"

#include "tinfra/fs.h"

#include "tinfra/fmt.h"
#include "tinfra/os_common.h"
#include "tinfra/path.h"

#include <cstring>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_IO_H
#include <io.h>
#endif
#include <stdio.h>

#ifdef HAVE_OPENDIR
#include <dirent.h>
#endif
    
#include "tinfra/trace.h"

namespace tinfra {
namespace fs { 

struct lister::internal_data {
    DIR* handle;
};

lister::lister(tstring const& path, bool need_stat)
    : data_(new internal_data())
{   
    tinfra::string_pool temporary_context;
    TINFRA_TRACE_MSG("lister::lister");    
    TINFRA_TRACE_VAR(path);
    data_->handle = ::opendir(path.c_str(temporary_context));
    if( !data_->handle ) {
        throw_errno_error(errno, fmt("unable to read dir '%s'") % path);
    }
}
lister::~lister()
{
    TINFRA_TRACE_MSG("lister::~lister");
    if( data_->handle  != 0 )
        ::closedir(data_->handle);
}

bool lister::fetch_next(directory_entry& result)
{
    TINFRA_TRACE_MSG("lister::fetch_next");
    
    while( true ) {
        dirent* entry = ::readdir(data_->handle);
        if( entry == 0 )
            return false;
        
        if( std::strcmp(entry->d_name,"..") == 0 || 
            std::strcmp(entry->d_name,".") == 0 ) 
        {
            continue;
        }
        
        result.name = entry->d_name;
        TINFRA_TRACE_VAR(result.name);
        result.info.is_dir = false;
        result.info.size = 0;
        
        // TODO, recognize how to use FILETIME and how to convert it into timestamp
        result.info.modification_time = 0;
        result.info.access_time = 0;
        return true;
    }
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
    int result = ::mkdir(name.c_str(temporary_context), 0777);
    if( result == -1 ) {
        throw_errno_error(errno, fmt("unable to create dir '%s'") % name);
    }
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
    
} } // end namespace tinfra::fs
