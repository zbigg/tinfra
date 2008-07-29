#ifndef __tinfra_vfs_h__
#define __tinfra_vfs_h__

#include "tinfra/fs.h"
#include "tinfra/io.h"

namespace tinfra {

class vfs {
public:
    virtual ~vfs() {}
    
    virtual std::string root_url() const = 0;
    
    virtual void list_files(const char* path, file_list_visitor& visitor) = 0;
    
    //virtual void stat(const char* path, stat&) = 0;
    
    virtual tinfra::io::stream* open(const char* path, tinfra::io::openmode mode) = 0;

    virtual void rm(const char* name) = 0;

    virtual void rmdir(const char* name) = 0;
    
    virtual void copy(const char* src, const char* dest) = 0;

    virtual void mv(const char* src, const char* dst) = 0;
    
    virtual void recursive_copy(const char* src, const char* dest) = 0;
    
    virtual void recursive_rm(const char* src) = 0;
};

} // end namespace tinfra

#endif
