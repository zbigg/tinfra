//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_vfs_h__
#define __tinfra_vfs_h__

#include "tinfra/fs.h"
#include "tinfra/io/stream.h"

namespace tinfra {

class vfs {
public:
    virtual ~vfs() {}
    
    virtual tinfra::fs::file_name_list roots() = 0;
    
    virtual void list_files(const char* path, tinfra::fs::file_list_visitor& visitor) = 0;
    
    virtual tinfra::fs::file_info stat(const char* path) = 0;
    
    virtual tinfra::io::stream* open(const char* path, tinfra::io::openmode mode) = 0;

    virtual void rm(const char* name) = 0;

    virtual void rmdir(const char* name) = 0;
    
    virtual void mkdir(const char* name) = 0;
        
    virtual void copy(const char* src, const char* dest) = 0;

    virtual void mv(const char* src, const char* dst) = 0;
    
    virtual void recursive_copy(const char* src, const char* dest) = 0;
    
    virtual void recursive_rm(const char* src) = 0;
        
    virtual bool is_file(const char* name) = 0;
    virtual bool is_dir(const char* name) = 0;    
    virtual bool exists(const char* name) = 0;
};

class generic_vfs: public vfs {
public:
    using vfs::list_files;
    virtual void list_files(const char* path, tinfra::fs::file_name_list& result);
    
    virtual void copy(const char* src, const char* dest);
    
    virtual void recursive_copy(const char* src, const char* dest);
    
    virtual void recursive_rm(const char* src);
    
    virtual bool is_file(const char* name);
    virtual bool is_dir(const char* name);
    virtual bool exists(const char* name);
};

tinfra::vfs& local_fs();

} // end namespace tinfra

#endif
