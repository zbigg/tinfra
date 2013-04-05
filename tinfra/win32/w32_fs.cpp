//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "../platform.h"

#ifdef TINFRA_W32

#include "tinfra/fs.h"

#include "tinfra/win32.h"
#include "tinfra/path.h"
#include "tinfra/fmt.h"
#include "tinfra/cmd.h" // for getapp

#include <string>
#include <cctype>
#include <stdio.h>
#include <windows.h>

#include "tinfra/trace.h"

namespace tinfra {
namespace fs { 

tinfra::module_tracer win32_fs_tracer(tinfra::tinfra_tracer, "win32_fs", 70);

struct lister::internal_data {
    
    HANDLE ff_handle;
    WIN32_FIND_DATAW ff_data;
    bool         is_first;
    std::string  name_holder;
};

lister::lister(tstring const& path, bool /*need_stat*/)
    : data_(new internal_data())
{   
    TINFRA_TRACE(win32_fs_tracer, "lister::lister");    
    TINFRA_TRACE_VAR(win32_fs_tracer, path);
    std::wstring wname = tinfra::win32::make_wstring_from_utf8(path);
    wname.append(L"\\*.*");
    data_->ff_handle = FindFirstFileW(wname.c_str(), &data_->ff_data );
    
    if( data_->ff_handle == INVALID_HANDLE_VALUE ) {
        tinfra::win32::throw_system_error(fmt("unable to list directory '%s'") % path);
    }
    data_->is_first = true;
}
lister::~lister()
{
    TINFRA_TRACE(win32_fs_tracer, "lister::~lister");
    if( data_->ff_handle != INVALID_HANDLE_VALUE )
        FindClose(data_->ff_handle);
}

bool lister::fetch_next(directory_entry& result)
{
    TINFRA_TRACE(win32_fs_tracer, "lister::fetch_next");
    
    while( true ) {
        bool have_data = data_->is_first;
        data_->is_first = false;
        if( !have_data )
            have_data =  (FindNextFileW(data_->ff_handle, &data_->ff_data) != 0 );
        if( !have_data )
            return false;       
        
        WIN32_FIND_DATAW& ff_data = data_->ff_data;
        if( ff_data.cFileName[0] == '.' ) 
        {
            if( (ff_data.cFileName[1] == 0) ||
                (ff_data.cFileName[1] == '.' && ff_data.cFileName[2] == 0) )
            {
                continue;
            }
        }
        
        data_->name_holder = tinfra::win32::make_utf8(ff_data.cFileName);
        
        result.name = data_->name_holder;
        TINFRA_TRACE_VAR(win32_fs_tracer, result.name);
        result.info.is_dir = (ff_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == FILE_ATTRIBUTE_DIRECTORY;
        result.info.type = result.info.is_dir ? DIRECTORY : REGULAR_FILE;
        result.info.size = size_t(ff_data.nFileSizeLow);
        // TODO: add support for win64 build
        // result.info.size |= (size_t(ff_data.nFileSizeHigh) << 32);
        
        // TODO, recognize how to use FILETIME and how to convert it into timestamp
        result.info.modification_time = 0;
        result.info.access_time = 0;
        data_->is_first = false;
        return true;
    }
}

file_info stat(tstring const& name)
{
    const std::wstring w_name = tinfra::win32::make_wstring_from_utf8(name);
    
    WIN32_FILE_ATTRIBUTE_DATA file_attrs;
    
    const BOOL get_result = GetFileAttributesExW(
        w_name.c_str(), 
        GetFileExInfoStandard,
        &file_attrs);
    if( get_result == 0) {
        tinfra::win32::throw_system_error(fmt("unable to get file information for '%s' (GetFileAttributesExW)") % name);
    }
    
    file_info result;
    result.is_dir = (file_attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
    result.modification_time = 0; // TODO: how to convert FILETIME to epoch time
    result.access_time = 0;  // TODO: how to convert FILETIME to epoch time
    result.size = file_attrs.nFileSizeLow; 
	result.type = result.is_dir ? DIRECTORY : REGULAR_FILE;
    // TODO: add support for win64 build
    // result.size |= (size_t(file_attrs.nFileSizeHigh) << 32);
    return result;
}

bool exists(tstring const& name)
{
    const std::wstring w_name = tinfra::win32::make_wstring_from_utf8(name);
    const DWORD dwResult = GetFileAttributesW(w_name.c_str());
    
    if( dwResult == INVALID_FILE_ATTRIBUTES )
        return false;
    else
        return true;
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
    const std::wstring w_name = tinfra::win32::make_wstring_from_utf8(name);
    const DWORD dwResult = GetFileAttributesW(w_name.c_str());
    
    if( dwResult == INVALID_FILE_ATTRIBUTES )
        return false;
    if( (dwResult & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
        return true;
    return false;
}

bool is_file(tstring const& name)
{
    const std::wstring w_name = tinfra::win32::make_wstring_from_utf8(name);
    const DWORD dwResult = GetFileAttributesW(w_name.c_str());
    
    if( dwResult == INVALID_FILE_ATTRIBUTES )
        return false;
    if( (dwResult & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
        return false;
    return true;
}

void mv(tstring const& src, tstring const& dest)
{
    std::wstring w_src = tinfra::win32::make_wstring_from_utf8(src);
    std::wstring w_dest = tinfra::win32::make_wstring_from_utf8(dest);
    const BOOL result = ::MoveFileExW(
        w_src.c_str(),
        w_dest.c_str(),
        MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
        
    if( result == 0) {
        tinfra::win32::throw_system_error(fmt("unable to move file from '%s' to '%s' (MoveFileExW)") % src % dest);
    }
}

void rm(tstring const& name)
{
    // TODO: according to MSDN, we should remove read-only attribute before deletion (?)
    std::wstring w_name = tinfra::win32::make_wstring_from_utf8(name);
	const BOOL result = ::DeleteFileW(w_name.c_str());
	if( result == 0 ) {
		tinfra::win32::throw_system_error(fmt("unable to remove file '%s' (DeleteFileW)") % name);
	}
}

void rmdir(tstring const& name)
{
    std::wstring w_name = tinfra::win32::make_wstring_from_utf8(name);
    const BOOL result = ::RemoveDirectoryW(w_name.c_str());
    if( result == 0 ) {
        tinfra::win32::throw_system_error(fmt("unable to remove folder '%s' (RemoveDirectoryW)") % name);
    }
}

void mkdir(tstring const& name, bool create_parents)
{
    using tinfra::path::dirname;
    std::string parent = dirname(name);
    if( !is_dir(parent) ) {
        if( create_parents )
            mkdir(parent);
        else {
			const std::string message = (fmt("unable to create dir '%s': %s") % name % "parent folder doesn't exist").str();
            throw std::logic_error(message);
		}
    }
    
    const std::wstring w_name = tinfra::win32::make_wstring_from_utf8(name);
    const BOOL result = ::CreateDirectoryW(w_name.c_str(), NULL);
    if( result == 0 ) {
        tinfra::win32::throw_system_error(fmt("unable to create dir '%s' (CreateDirectoryW)") % name);
    }
}

/*
TODO
   Those are common to all platforms. Maybe rewrite them in future
   using WinAPI.
*/
/*
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
*/

void cd(tstring const& name)
{
    const std::wstring w_name = tinfra::win32::make_wstring_from_utf8(name);
    const BOOL result = ::SetCurrentDirectoryW(w_name.c_str());
    if( result == 0 ) {
        tinfra::win32::throw_system_error(fmt("unable to change current directory to '%s' (SetCurrentDirectoryW)") % name);
    }
}

std::string pwd()
{
    const int BUFFER_LEN=2048;
    WCHAR buf[BUFFER_LEN];
    DWORD size = ::GetCurrentDirectoryW(BUFFER_LEN, buf);
    if( size == 0 ) {
        tinfra::win32::throw_system_error("unable to get current directory (GetCurrentDirectoryW)");
    }
    if( size < BUFFER_LEN )  {
        std::wstring w_result(buf, size);
        std::string result = tinfra::win32::make_utf8(w_result.c_str());
        return result;
    }
    tinfra::win32::throw_system_error("fs::pwd() failed (GetCurrentDirectoryW): implement it better!");
    return ""; // just to satisfy compiler
}

//void         symlink(tstring const& target, tstring const& path)
//{    
//}

//std::string readlink(tstring const& path)
//{
//    
//}

std::string realpath(tstring const& path)
{    
    return path;
}

} } // end namespace tinfra::fs

#endif // TINFRA_W32

