//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/fs.h"
#include "tinfra/win32.h"
#include "tinfra/fmt.h"

#include <string>
#include <stdio.h>
#include <windows.h>

#include "tinfra/trace.h"

namespace tinfra {
namespace fs { 

struct lister::internal_data {
    
    HANDLE ff_handle;
    WIN32_FIND_DATAW ff_data;
    bool         is_first;
    std::string  name_holder;
};

lister::lister(tstring const& path, bool need_stat)
    : data_(new internal_data())
{   
    TINFRA_TRACE_MSG("lister::lister");    
    TINFRA_TRACE_VAR(path);
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
    TINFRA_TRACE_MSG("lister::~lister");
    if( data_->ff_handle != INVALID_HANDLE_VALUE )
        FindClose(data_->ff_handle);
}

bool lister::fetch_next(directory_entry& result)
{
    TINFRA_TRACE_MSG("lister::fetch_next");
    
    while( true ) {
        bool have_data = data_->is_first;
        data_->is_first = false;
        if( !have_data )
            have_data =  FindNextFileW(data_->ff_handle, &data_->ff_data);
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
        TINFRA_TRACE_VAR(result.name);
        result.info.is_dir = (ff_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == FILE_ATTRIBUTE_DIRECTORY;
        result.info.size = size_t(ff_data.nFileSizeLow) | (size_t(ff_data.nFileSizeHigh) << 32);
        
        // TODO, recognize how to use FILETIME and how to convert it into timestamp
        result.info.modification_time = 0;
        result.info.access_time = 0;
        data_->is_first = false;
        return true;
    }
}
    
    
} } // end namespace tinfra::fs
