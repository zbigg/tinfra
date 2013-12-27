//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/vfs.h"
#include "tinfra/fs.h"
#include "tinfra/path.h"

#include <stdexcept>

#ifdef WIN32
#include "tinfra/win32.h"
#endif

namespace tinfra {

class shell {
    typedef std::vector<std::string> command;
};

shell::command& operator << (shell::command& c, std::string const& s) { c.push_back(s); }

class vfs_sh: public generic_vfs {

    void copy(const char* src, const char* dest)
    {
        shell::command c;
        c << "cp" << "-f" << src << dest;
        
        shell::result r;
        s.execute(c,r);
        
        if( r.exit_code() != 0 ) {
            throw std::runtime_error(r.get_error());
        }
    }


    void recursive_copy(const char* src, const char* dest)
    {
        shell::command c;
        c << "cp" << "-fr" << src << dest;
        
        shell::result r;
        s.execute(c,r);
        
        if( r.exit_code() != 0 ) {
            throw std::runtime_error(r.error_string());
        }
    }

    void recursive_rm(const char* name)
    {
        shell::command c;
        c << "rm" << "-rf" << name;
        
        shell::result r;
        s.execute(c,r);
        
        if( r.exit_code() != 0 ) {
            throw std::runtime_error(r.error_string());
        }
    }

    bool is_file(const char* name)
    {
        shell::command c;
        c << "test" << "-f" << name;
        
        shell::result r;
        s.execute(c,r);
        
        return r.exit_code() == 0;
    }

    bool is_dir(const char* name)
    {
        shell::command c;
        c << "test" << "-d" << name;
        
        shell::result r;
        s.execute(c,r);
        
        return r.exit_code() == 0;
    }

    bool generic_vfs::exists(const char* name)
    {
        shell::command c;
        c << "test" << "-e" << name;
        
        shell::result r;
        s.execute(c,r);
        
        return r.exit_code() == 0;
    }

    tinfra::fs::file_name_list roots() const {
        tinfra::fs::file_name_list result;
        result.push_back("/");
        return result;
    }
    
    void list_files(const char* path, tinfra::fs::file_list_visitor& visitor)
    {
        shell::command c;
        c << "ls" << "-1" << path;
        
        shell::result r;
        s.execute(c,r);
        
        // TODO api for reading lines
    }
    
    tinfra::fs::file_info stat(const char* path)
    {
        shell::command c;
        c << "stat" << "--format" << "%s\n%F\n%Y\n%X\n" << name;
        
        shell::result r;
        s.execute(c,r);
        
        return r.exit_code() == 0;
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
