//
// Copyright (c) 2009,2013, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/file.h" // we implement this
#include "../platform.h"

#ifdef TINFRA_W32

#include "tinfra/fmt.h"
#include "tinfra/win32.h"

#include <stdio.h>
#include <windows.h>

namespace tinfra {

static void throw_get_last_error(const char* message)
{
    tinfra::win32::throw_system_error(message);
}

file::file(handle_type h):
    handle_(h)
{
}

file::file(tstring const& name, int flags):
    handle_(-1)
{
    const bool fread  = (flags & FOM_READ) == FOM_READ;
    const bool fwrite = (flags & FOM_WRITE) == FOM_WRITE;
    
    DWORD dwDesiredAccess = 0;
    
    if( fread )   dwDesiredAccess |= GENERIC_READ;
    if( fwrite  ) dwDesiredAccess |= GENERIC_WRITE;
        
    DWORD dwCreationDistribution;
    /* convert mode to win32 native, taken from zcompat */
    {
        const bool fcreat = (flags & FOM_CREATE) == FOM_CREATE; // XXX: is it always true in ios ?
                                    // for now we create if we want write
        const bool fexcl = false;   // XXX: should we support it, ios supports it ?
        const bool ftrunc = (flags & FOM_TRUNC) == FOM_TRUNC;
        
        if( fcreat ) {
            if( fexcl )
                if( ftrunc )    /* fcreat | fexcl | ftrunc */
                    dwCreationDistribution = CREATE_NEW;
                else           /* fcreat | fexcl ----  BAD */
                    dwCreationDistribution = 0;
            else
                if( ftrunc )    /* fcreat | ftrunc */
                    dwCreationDistribution = CREATE_ALWAYS;
                else            /* fcreat */
                    dwCreationDistribution = OPEN_ALWAYS;
        } else if( ftrunc ) {
            if( fexcl )             /* ftrunc | fexcl  ---- BAD */
                dwCreationDistribution = 0;
            else                    /* ftrunc */
                dwCreationDistribution = TRUNCATE_EXISTING;
        } else if( fexcl ) {            /* fexcl ----  BAD*/
            dwCreationDistribution = 0;
        } else
            dwCreationDistribution = OPEN_EXISTING;
    }
    std::wstring w_name = tinfra::win32::make_wstring_from_utf8(name);
    this->handle_ = (intptr_t)CreateFileW(w_name.c_str(),
                dwDesiredAccess,
                0,
                NULL,
                dwCreationDistribution,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
    if( (HANDLE)this->handle_ == INVALID_HANDLE_VALUE || this->handle_ == 0 ) {
        throw_get_last_error(fmt("unable to open %s") % name);
    }
}

static int close_nothrow(HANDLE h)
{
    int rc = ::CloseHandle(h);
    return (rc == 0) ? -1 : 0;
}

file::~file()
{
    if( (HANDLE)this->handle_ != INVALID_HANDLE_VALUE ) {
        if( close_nothrow((HANDLE)this->handle_) == -1 ) {
            // TODO: add silent failures reporting
            // int err = get_last_error();
            // tinfra::silent_failure(fmt("file close failed: %i" % blabla )
        }
        release();
    }
}

void file::close()
{
    if( (HANDLE)this->handle_ != INVALID_HANDLE_VALUE ) {
        if( close_nothrow((HANDLE)this->handle_) == -1 ) { 
            throw_get_last_error("close failed");
        }
    }
    release();
}

int file::seek(int pos, base_file::seek_origin origin)
{
    DWORD native_origin = 0;
    
    switch( origin ) {
    case SO_START:
        native_origin = FILE_BEGIN;
        break;
    case SO_CURRENT:
        native_origin = FILE_CURRENT;
        break;
    case SO_END:
        native_origin = FILE_END;
        break;
    }
    DWORD r = SetFilePointer((HANDLE)this->handle_, (LONG) pos, NULL, native_origin);
    if( r != 0xffffffff ) {
        return (int)r;
    } else {
        throw_get_last_error("seek failed");
        // doesn't return
        return -1;
    }
}

int file::read(char* data, int size)
{
    DWORD readed;
    if( ReadFile((HANDLE)this->handle_,
                (LPVOID)data,
                (DWORD) size,
                &readed,
                NULL) == 0)
    {
        DWORD error = ::GetLastError();
        if( error == ERROR_BROKEN_PIPE )
            return 0;
        throw_get_last_error("read failed"); 
    }
    //printf("win32_stream: readed %i bytes\n", readed);
    return readed;
}

int file::write(char const* data, int size)
{
    DWORD written;
    if( WriteFile((HANDLE)this->handle_,
                  (LPCVOID)data,
                  (DWORD)  size,
                  &written,
                  NULL ) == 0 ) 
    {        
        throw_get_last_error("write failed"); 
    }
    //printf("win32_stream: written %i bytes\n", written);
    return written;
}

fs::file_info file::stat()
{
    throw std::logic_error("file::stat() not implemented");   
}

void file::sync()
{
}

// move to native handle ??
intptr_t file::native() const
{
    return this->handle_;
}
void     file::release()
{
    this->handle_ = (intptr_t)INVALID_HANDLE_VALUE;
}
   
}// end namespace tinfra::io

#endif // TINFRA_W32

