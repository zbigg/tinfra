//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifdef __linux__
#define _FILE_OFFSET_BITS 64
#endif

#include "../platform.h"

#ifdef TINFRA_POSIX

#include "tinfra/fs.h"

#include "tinfra/fmt.h"
#include "tinfra/fail.h"
#include "tinfra/os_common.h"
#include "tinfra/path.h"
#include "tinfra/logger.h"
#include "tinfra/trace.h"


#include <cstring>
#include <errno.h>
#include <unistd.h> // for readlink, symlink 
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

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

tinfra::module_tracer fs_tracer(tinfra::tinfra_tracer, "fs");

struct lister::internal_data {
    std::string base_path;
    DIR* handle;
    bool need_stat;
};

lister::lister(tstring const& path, bool need_stat)
    : data_(new internal_data())
{
    data_->base_path = path.str();
    data_->need_stat = need_stat;
    tinfra::string_pool temporary_context;
    TINFRA_TRACE(fs_tracer, "lister::lister");    
    TINFRA_TRACE_VAR(fs_tracer, path);
    data_->handle = ::opendir(path.c_str(temporary_context));
    if( !data_->handle ) {
        tinfra::fail(fmt("unable to open directory '%s' for listing") % path, errno_to_string(errno));
    }
}
lister::~lister()
{
    TINFRA_TRACE(fs_tracer, "lister::~lister");
    if( data_->handle  != 0 )
        ::closedir(data_->handle);
}

bool lister::fetch_next(directory_entry& result)
{
    TINFRA_TRACE(fs_tracer, "lister::fetch_next");
    
    while( true ) {
        errno = 0;
        dirent* entry = ::readdir(data_->handle);
        if( entry == 0 ) {
            if( errno != 0 ) {
                tinfra::fail("failed read directory contents", errno_to_string(errno));
            }
            return false;
        }
        
        if( std::strcmp(entry->d_name,"..") == 0 || 
            std::strcmp(entry->d_name,".") == 0 ) 
        {
            continue;
        }
        
        result.name = entry->d_name;
        TINFRA_TRACE_VAR(fs_tracer, result.name);
        
        if( data_->need_stat ) {
            std::string path = path::join(data_->base_path, result.name);
            result.info = stat(path);
            result.info.is_dir = (result.info.type == DIRECTORY);
        } else {
            result.info.is_dir = false;
            result.info.size = 0;
            result.info.type = REGULAR_FILE; // as fallback
            result.info.modification_time = 0;
            result.info.access_time = 0;
        }
        return true;
    }
}

file_info stat(tstring const& name)
{
    string_pool temporary_context;
    struct stat st;
    if( ::lstat(name.c_str(temporary_context), &st) != 0 ) {
        tinfra::fail(fmt("unable stat file '%s'") % name, errno_to_string(errno));
    }
    
    file_info result;
    
    int file_type = st.st_mode & S_IFMT; 
    if ( file_type == S_IFLNK )
    	result.type = SYMBOLIC_LINK;
    else if( file_type == S_IFDIR )
    	result.type = DIRECTORY;    
    else if( (file_type == S_IFCHR)  || 
    	     (file_type == S_IFBLK) )
    	result.type = DEVICE;
    else if ( file_type == S_IFIFO )
    	result.type = FIFO;
    else if ( file_type == S_IFSOCK) 
        result.type = SOCKET;
    else
    	result.type = REGULAR_FILE;

    result.is_dir = (result.type == DIRECTORY);
    
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
    int file_type = st.st_mode & S_IFMT;
    bool result = (file_type == S_IFDIR);
    return result;
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
        tinfra::fail(fmt("unable to rename from '%s' to '%s' ") % src % dest, errno_to_string(errno));
    }
}

void rm(tstring const& name)
{
    string_pool temporary_context;
    int result = ::unlink(name.c_str(temporary_context));
    if( result == -1 ) {
        tinfra::fail(fmt("unable to remove file '%s'") % name, errno_to_string(errno));
    }
}

void rmdir(tstring const& name)
{
    string_pool temporary_context;
    int result = ::rmdir(name.c_str(temporary_context));
    if( result == -1 ) {
        tinfra::fail(fmt("unable to remove directory '%s'") % name, errno_to_string(errno));
    }
}

void mkdir(tstring const& name, bool create_parents)
{
    std::string parent = path::dirname(name);
    if( !is_dir(parent) ) {
        if( create_parents ) {
            mkdir(parent);
        } else {
            tinfra::fail(fmt("unable to create dir '%s': %s") % name, "parent folder doesn't exist");
        }
    }
    string_pool temporary_context;
    int result = ::mkdir(name.c_str(temporary_context), 0777);
    if( result == -1 ) {
        tinfra::fail(fmt("unable to create dir '%s'") % name, errno_to_string(errno));
    }
}


void cd(tstring const& dirname)
{
    string_pool temporary_context;
    int result = ::chdir(dirname.c_str(temporary_context));
    if( result == -1 ) {
        tinfra::fail(fmt("unable change directory to '%s'") % dirname, errno_to_string(errno));
    }
}

std::string pwd()
{
    char buf[1024];
    if( getcwd(buf, sizeof(buf)) == 0 ) {
        tinfra::fail("fs::pwd() unable read pwd (implement it better!)", errno_to_string(errno));
    }
    return std::string(buf);
}

void         symlink(tstring const& target, tstring const& path)
{
    string_pool pool;
    const int r = ::symlink(target.c_str(pool), path.c_str(pool));
    if( r == -1 ) {
        tinfra::fail(fmt("symlink('%s', '%s') failed") % target % path, errno_to_string(errno));
    }
}

std::string readlink(tstring const& path)
{
    string_pool pool;
    char buf[1024];
    const int r = ::readlink(path.c_str(pool), buf, sizeof(buf));
    if( r == -1 ) {
        tinfra::fail(fmt("readlink('%s') failed") % path, errno_to_string(errno));
    }
    buf[r] = 0;
    return buf;
}

std::string realpath(tstring const& path)
{
    string_pool pool;
    std::string result;
    
    char* x = ::realpath(path.c_str(pool), NULL);
    if( !x ) {
        tinfra::fail(fmt("realpath('%s') failed") % path, errno_to_string(errno));
    }
    result = x;
    ::free(x);
    return result;
}


} } // end namespace tinfra::fs

#endif // TINFRA_POSIX

