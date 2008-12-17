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
    
    virtual void list_files(tstring const& path, tinfra::fs::file_list_visitor& visitor) = 0;
    
    virtual tinfra::fs::file_info stat(tstring const& path) = 0;
    
    virtual tinfra::io::stream* open(tstring const& path, tinfra::io::openmode mode) = 0;

    virtual void rm(tstring const& name) = 0;

    virtual void rmdir(tstring const& name) = 0;
    
    virtual void mkdir(tstring const& name) = 0;
        
    virtual void copy(tstring const& src, tstring const& dest) = 0;

    virtual void mv(tstring const& src, tstring const& dst) = 0;
    
    virtual void recursive_copy(tstring const& src, tstring const& dest) = 0;
    
    virtual void recursive_rm(tstring const& src) = 0;
        
    virtual bool is_file(tstring const& name) = 0;
    virtual bool is_dir(tstring const& name) = 0;    
    virtual bool exists(tstring const& name) = 0;
};

class generic_vfs: public vfs {
public:
    using vfs::list_files;
    virtual void list_files(tstring const& path, tinfra::fs::file_name_list& result);
    
    virtual void copy(tstring const& src, tstring const& dest);
    
    virtual void recursive_copy(tstring const& src, tstring const& dest);
    
    virtual void recursive_rm(tstring const& src);
    
    virtual bool is_file(tstring const& name);
    virtual bool is_dir(tstring const& name);
    virtual bool exists(tstring const& name);
};

tinfra::vfs& local_fs();

// generic folder algroithms

/// Ensure that folder exists
///
/// Creates folder and possibly it's parent to satisfy postcondition.
///
/// Poscontidion: fs contains folder path
void ensure_dir_exists(tinfra::vfs& fs, tstring const& path);

/// Recursive copy between filesystems.
///
/// If sfs and dfs are same may use fs internal copying mechanisms
void recursive_copy(tinfra::vfs& sfs, tstring const& source,
                    tinfra::vfs& dfs, tstring const& dest);
/// Copy files between filesystems.
///
/// If sfs and dfs are same may use fs internal copying mechanisms
void copy(tinfra::vfs& sfs, tstring const& source,
          tinfra::vfs& dfs, tstring const& dest);


void list_files(tinfra::vfs& fs, tstring const& path, tinfra::fs::file_name_list& result);

/// Recursive copy file or folder
///
/// In case of file default_copy is performed.
/// In case of folder recursive copy is performed.
///
/// Doesn't check sfs type, sfs/dfs identity.
///
/// Use only FS primitive calls (is_dir, list_files, open_file). 
void default_recursive_copy(tinfra::vfs& sfs, tstring const& src,
                            tinfra::vfs& dfs, tstring const& dest);

/// copy file byte per byte
///
/// Doesn't check sfs type, sfs/dfs identity.
///
/// Works only for regular files.
/// Uses only FS primitives: is_file and open_file.
void default_copy(tinfra::vfs& sfs, tstring const& src,
                  tinfra::vfs& dfs, tstring const& dest);

/// recursively remove file/folder
///
/// Use default algorithm for folder discovery and
/// removal of all it's elements.
///
/// Use only FS primitive calls (is_dir, list_files, rmdir, rm). 
void default_recursive_rm(tinfra::vfs& fs, tstring const& name);

} // end namespace tinfra

#endif

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

