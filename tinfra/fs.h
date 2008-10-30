//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_fs_h__
#define __tinfra_fs_h__

#include <string>
#include <vector>

#include "tinfra/tstring.h"

namespace tinfra {
namespace fs {

typedef std::vector<std::string> file_name_list;
    
struct file_list_visitor {
    virtual void accept(tstring const& name) =0;
};

void list_files(tstring const& path, file_list_visitor& visitor);
void list_files(tstring const& path, std::vector<std::string>& result);
inline std::vector<std::string> list_files(const char* path) {
    std::vector<std::string> r;
    list_files(path, r);
    return r;
}

struct file_info {
    size_t    size;
    bool      is_dir;   
    time_t    modification_time;
    time_t    access_time;
};

file_info stat(tstring const& name);

void cd(tstring const& dirname);
std::string pwd();

void mkdir(tstring const& name, bool create_parents = true);

void copy(tstring const& src, tstring const& dest);
void mv(tstring const& src, tstring const& dest);

void recursive_copy(tstring const& src, tstring const& dest);

void rm(tstring const& name);
void rmdir(tstring const& name);
void recursive_rm(tstring const& src);

struct walker 
{
    /** Throw this from accept to stop walk. */
    struct stop { };
    
    /** 
        Accept element.
    
        @return callback should return if it's interested in walking
                through children of current element 
        @throws throws walker::stop to forcifully stop walking
    */
    virtual bool accept(tstring const& name, tstring const& parent, bool is_dir) = 0;
};

/** Walk through filesystem hierarchy.

*/
void walk(tstring const& start, walker& w);

} }

#endif

