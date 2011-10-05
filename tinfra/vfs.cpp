//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/vfs.h"
#include "tinfra/fs.h"
#include "tinfra/path.h"
#include "tinfra/tstring.h"

#include <stdexcept>
#include <memory>

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
        
    virtual void accept(tstring const& name)
    {
        result.push_back(name.str());
    }
};

void generic_vfs::list_files(tstring const& path, std::vector<std::string>& result)
{
    vector_sink_file_visitor2 visitor(result);
    list_files(path, visitor);
}

void generic_vfs::copy(tstring const& src, tstring const& dest)
{
    default_copy(*this, src, *this, dest);
}


void generic_vfs::recursive_copy(tstring const& src, tstring const& dest)
{
    default_recursive_copy(*this, src, *this, dest);
}

void generic_vfs::recursive_rm(tstring const& name)
{
    default_recursive_rm(*this, name);
}

bool generic_vfs::is_file(tstring const& name)
{
    try {
        
        return ! stat(name).is_dir;
        
        // TODO, remove exception flow -> should use exception-less stat here
    } catch( std::exception& ) {
        return false;
    }
}

bool generic_vfs::is_dir(tstring const& name)
{
    try {
        
        return stat(name).is_dir;
        
        // TODO, remove exception flow -> should use exception-less stat here
    } catch( std::exception&  ) {
        return false;
    }
}

bool generic_vfs::exists(tstring const& name)
{
    try {
        stat(name);
        return true;
        // TODO, remove exception flow -> should use exception-less stat here
    } catch( std::exception&  ) {
        return false;
    }
}

//
// basic_local_fs
//

class basic_local_fs: public generic_vfs {
public:
    tinfra::fs::file_name_list roots() {
        tinfra::fs::file_name_list result;
#ifdef WIN32
        tinfra::win32::get_available_drives(result);
#else
        result.push_back("/");
#endif
        return result;
    }
    
    void list_files(tstring const& path, tinfra::fs::file_list_visitor& visitor)
    {
        tinfra::fs::list_files(path, visitor);
    }
    
    tinfra::fs::file_info stat(tstring const& path)
    {
        return tinfra::fs::stat(path);
    }
    
    tinfra::io::stream* open(tstring const& path, tinfra::io::openmode mode)
    {
        string_pool temp_pool;
        return tinfra::io::open_file(path.c_str(temp_pool), mode);
    }

    void mkdir(tstring const& name)
    {
        tinfra::fs::mkdir(name);
    }
    void rm(tstring const& name) 
    {
        tinfra::fs::rm(name);
    }

    void rmdir(tstring const& name)
    {
        tinfra::fs::rmdir(name);
    }

    void mv(tstring const& src, tstring const& dst) 
    {
        tinfra::fs::mv(src, dst);
    }
    
    // implemented in generic_vfs
    // void copy(tstring const& src, tstring const& dest); 
    // void recursive_copy(tstring const& src, tstring const& dest);    
    // void recursive_rm(tstring const& src) = 0;
        
    virtual bool is_file(tstring const& name) { return tinfra::fs::is_file(name); }
    virtual bool is_dir(tstring const& name)  { return tinfra::fs::is_dir(name); }
    virtual bool exists(tstring const& name)  { return tinfra::fs::exists(name); }
};

tinfra::vfs& local_fs()
{
    static basic_local_fs instance;
    return instance;
}

// generic folder algroithms

void ensure_dir_exists(tinfra::vfs& fs, tstring const& name)
{
    if( fs.is_dir(name) ) {
         return;
    }
    
    std::string parent = path::dirname(name);
    if( ! fs.is_dir(parent) ) {
        ensure_dir_exists(fs, parent);        
    }
    fs.mkdir(name);
}

void recursive_copy(vfs& sfs, tstring const& src,
                    vfs& dfs, tstring const& dest)
{
    if( &sfs == &dfs ) {
        sfs.recursive_copy(src, dest);
        return;
    } 
    default_recursive_copy(sfs, src, dfs, dest);
}

void copy(vfs& sfs, tstring const& src,
          vfs& dfs, tstring const& dest)
{
    if( &sfs == &dfs ) {
        sfs.copy(src, dest);
        return;
    }
    default_copy(sfs, src, dfs, dest);
}

void list_files(tinfra::vfs& fs, tstring const& path, tinfra::fs::file_name_list& result)
{
    vector_sink_file_visitor2 visitor(result);
    fs.list_files(path, visitor);
}

void default_recursive_copy(vfs& sfs, tstring const& src,
                            vfs& dfs, tstring const& dest)
{
    // note, symlinks are not handled!
    if( dfs.is_dir(dest ) ) {
        std::string new_dest = path::join(dest, path::basename(src));
        return recursive_copy(sfs, src, dfs, new_dest);
    } else if( sfs.is_dir(src) ) {
        dfs.mkdir(dest);
        std::vector<std::string> files;
        list_files(sfs, src, files);
        for( std::vector<std::string>::const_iterator i = files.begin(); i!=files.end(); ++i )
        {
            std::string const& entry_name = *i;
            std::string new_src = path::join(src, entry_name);
            std::string new_dest = path::join(dest, entry_name);
            recursive_copy(sfs, new_src, dfs, new_dest);
        }
    } else {
        copy(sfs, src, dfs, dest);
    }
}

void default_copy(vfs& sfs, tstring const& src,
                  vfs& dfs, tstring const& dest)
{
    if( sfs.is_dir(src) ) 
        throw std::invalid_argument("tinfra::default_copy supports only regular files");

    if( dfs.is_dir(dest) ) {
        std::string new_dest = path::join(dest, path::basename(src) );
        copy(sfs, src, dfs, new_dest);
        return;
    }

    typedef std::auto_ptr<tinfra::io::stream> stream_ptr;
    
    stream_ptr in(sfs.open(src, std::ios::in | std::ios::binary));
    stream_ptr out(dfs.open(dest, std::ios::out | std::ios::trunc | std::ios::binary));
    
    tinfra::io::copy(in.get(), out.get());
    
    out->close();
    // TODO: does input close failure should provoke copy failure !???
    // in->close();
}

void default_recursive_rm(tinfra::vfs& fs, tstring const& name)
{
    // note: symlinks are trated as generic files (just removed)
    tinfra::fs::file_info fi = fs.stat(name);
    if( fi.type == tinfra::fs::DIRECTORY ) {
        std::vector<std::string> files;
        list_files(fs, name, files);
        
        for( std::vector<std::string>::const_iterator i = files.begin(); i!=files.end(); ++i ) 
        {
            default_recursive_rm( fs, path::join(name, *i) );
        }
        fs.rmdir(name);
    } else {        
        fs.rm(name);
    }
}
} // end namespace tinfra

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

