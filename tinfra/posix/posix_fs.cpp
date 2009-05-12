//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/platform.h"

#include "tinfra/fs.h"

#include "tinfra/fmt.h"
#include "tinfra/os_common.h"

#include <cstring>
#include <errno.h>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_IO_H
#include <io.h>
#endif

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
    
    
} } // end namespace tinfra::fs
