#include "tinfra/vfs.h"
#include "tinfra/fs.h"
#include "tinfra/path.h"

#include <stdexcept>

#ifdef WIN32
#include "tinfra/win32.h"
#endif

namespace tinfra {
    

//
// generic_vfs
//

// TODO: vector_sink_file_visitor is already in fs.cpp
struct vector_sink_file_visitor2: public tinfra::fs::file_list_visitor {
    std::vector<std::string>& result;
    vector_sink_file_visitor2(std::vector<std::string>& result): result(result) {}
        
    virtual void accept(const char* name)
    {
        result.push_back(name);
    }
};

void generic_vfs::list_files(const char* path, std::vector<std::string>& result)
{
    vector_sink_file_visitor2 visitor(result);
    list_files(path, visitor);
}

void generic_vfs::copy(const char* src, const char* dest)
{
    if( is_dir(src) ) 
        throw std::invalid_argument("vfs::copy supports only regular files");
    if( is_dir(dest) ) {
        copy(src, path::join(dest, path::basename(src) ).c_str());
        return;
    }

    typedef std::auto_ptr<tinfra::io::stream> stream_ptr;
    
    stream_ptr in(open(src, std::ios::in | std::ios::binary));
    stream_ptr out(open(dest, std::ios::out | std::ios::trunc | std::ios::binary));
    
    tinfra::io::copy(in.get(), out.get());
    
    out->close();
}


void generic_vfs::recursive_copy(const char* src, const char* dest)
{
    if( is_dir(dest) ) {
        std::string new_dest = path::join(dest, path::basename(src));
        return recursive_copy(src, new_dest.c_str());
    } else if( is_dir(src) ) {
        mkdir(dest);
        std::vector<std::string> files;
        list_files(src, files);
        for( std::vector<std::string>::const_iterator i = files.begin(); i!=files.end(); ++i )
        {
            std::string new_src = path::join(src, *i);
            std::string new_dest = path::join(dest, *i);
            recursive_copy(new_src.c_str(), new_dest.c_str());
        }
    } else {
        copy(src, dest);
    }
}

void generic_vfs::recursive_rm(const char* name)
{
    if( is_dir(name) ) {
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

bool generic_vfs::is_file(const char* name)
{
    try {
        
        return ! stat(name).is_dir;
        
    } catch( std::exception& e ) {
        return false;
    }
}

bool generic_vfs::is_dir(const char* name)
{
    try {
        
        return stat(name).is_dir;
        
    } catch( std::exception& e ) {
        return false;
    }
}

bool generic_vfs::exists(const char* name)
{
    try {        
        stat(name);
        return true;
    } catch( std::exception& e ) {
        return false;
    }
}

//
// basic_local_fs
//

class basic_local_fs: public generic_vfs {
public:
    tinfra::fs::file_name_list roots() const {
        tinfra::fs::file_name_list result;
#ifdef WIN32
        tinfra::win32::get_available_drives(result);
#else
        result.push_back("/");
#endif
        return result;
    }
    
    void list_files(const char* path, tinfra::fs::file_list_visitor& visitor)
    {
        tinfra::fs::list_files(path, visitor);
    }
    
    tinfra::fs::file_info stat(const char* path)
    {
        return tinfra::fs::stat(path);
    }
    
    tinfra::io::stream* open(const char* path, tinfra::io::openmode mode)
    {
        return tinfra::io::open_file(path, mode);
    }

    void mkdir(const char* name)
    {
        tinfra::fs::mkdir(name);
    }
    void rm(const char* name) 
    {
        tinfra::fs::rm(name);
    }

    void rmdir(const char* name)
    {
        tinfra::fs::rmdir(name);
    }

    void mv(const char* src, const char* dst) 
    {
        tinfra::fs::mv(src, dst);
    }
    
    // implemented in generic_vfs
    // void copy(const char* src, const char* dest); 
    // void recursive_copy(const char* src, const char* dest);    
    // void recursive_rm(const char* src) = 0;
        
    virtual bool is_file(const char* name) { return tinfra::path::is_file(name); }
    virtual bool is_dir(const char* name)  { return tinfra::path::is_dir(name); }
    virtual bool exists(const char* name)  { return tinfra::path::exists(name); }
};

tinfra::vfs& local_fs()
{
    static basic_local_fs instance;
    return instance;
}

} // end namespace tinfra
