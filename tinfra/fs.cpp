//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//


#include "tinfra/platform.h"
#include "config-priv.h"

#include "tinfra/fs.h"
#include "tinfra/path.h"
#include "tinfra/fmt.h"
#include "tinfra/os_common.h"
#include "tinfra/vfs.h"
#include "tinfra/trace.h"
#include <streambuf>
#include <fstream>
#include <stdexcept>
#include <ios>
#include <memory>

#include <sys/types.h>
#include <sys/stat.h>

#include <cstring>
#include <errno.h>

#if defined _WIN32
#include <direct.h>
#endif

#ifdef HAVE_IO_H
#include <io.h>
#endif

namespace tinfra {
namespace fs {

void list_files(tstring const& dirname, file_list_visitor& visitor)
{
    lister files(dirname);
    for(lister::iterator i = files.begin(); i != files.end(); ++i ) {
        directory_entry const& de = *i;
        
        visitor.accept(de.name);
    }
}
    

void list_files(tstring const& dirname, std::vector<std::string>& result)
{
    lister files(dirname);
    for(lister::iterator i = files.begin(); i != files.end(); ++i ) {
        directory_entry const& de = *i;
        
        result.push_back(de.name.str());
    }
}

struct recursive_lister::internal_data {
    // the stack
    std::vector<lister*>    listers;
    std::vector<std::string> base_paths;
    //
    std::string             last_path;
    bool                    last_was_dir;
    
    bool                    recurse_enabled;
};

recursive_lister::recursive_lister(tstring const& path, bool):
    self(new internal_data())
{
    self->listers.push_back(new lister(path, true));
    self->base_paths.push_back(path);
    
    // fake for first run
    self->last_was_dir = false;
    self->recurse_enabled = false;
}
recursive_lister::~recursive_lister()
{
    for(std::vector<lister*>::const_iterator i = self->listers.begin(); i != self->listers.end(); ++i ) {
        delete *i;
    }
}

bool recursive_lister::fetch_next(directory_entry& de)
{
    if( self->recurse_enabled && self->last_was_dir) {
        self->listers.push_back(new lister(self->last_path, true));
        self->base_paths.push_back(self->last_path);
    }
    self->recurse_enabled = true;
    while( ! self->listers.empty() ) {
        lister* current = self->listers.back();
        bool r = current->fetch_next(de);
        if( r ) {
            self->last_path = path::join(self->base_paths.back(), de.name);
            self->last_was_dir = (de.info.type == tinfra::fs::DIRECTORY);
            TINFRA_GLOBAL_TRACE_VAR(self->last_path);
            TINFRA_GLOBAL_TRACE_VAR(self->last_was_dir);
            TINFRA_GLOBAL_TRACE_VAR(de.info.type);
            de.name = self->last_path;
            return true;
        } else {
            delete current;
            self->listers.pop_back();
            self->base_paths.pop_back();
            self->last_was_dir = false;
        }
    }
    return false;
}

void recursive_lister::recurse(bool recurse)
{
    self->recurse_enabled = recurse;
}

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

namespace {
    
static void walk_(tstring const& start, walker& walker);
    
struct walker_file_visitor: public tinfra::fs::file_list_visitor {
    tstring const& parent_;
    walker& walker_;
    
    walker_file_visitor(tstring const& parent, walker& walker): parent_(parent), walker_(walker) {}
        
    void accept(tstring const& name)
    {
        std::string file_path(tinfra::path::join(parent_, name));
        bool dir = is_dir(file_path);
        bool dig_further = walker_.accept(name,  parent_, dir);
        if( dir && dig_further ) {
            walk_(file_path, walker_);
        }
    }
};

static void walk_(tstring const& start, walker& w)
{    
    walker_file_visitor visitor(start, w);
    try {
        tinfra::fs::list_files(start, visitor);
    } catch(std::runtime_error const&) {
        // TBD, it should be more intelligent!!
    }    
}
}

void walk(tstring const& start, walker& w)
{
    try 
    {
        walk_(start, w);
    } 
    catch(walker::stop) { }
}

file_list_visitor::~file_list_visitor()
{
}

walker::~walker()
{
}

} }

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

