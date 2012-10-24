//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_fs_h_included
#define tinfra_fs_h_included

#include "tinfra/tstring.h"
#include "tinfra/generator.h"

#include <string>
#include <vector>
#include <memory>

namespace tinfra {
namespace fs {

enum file_type {
    REGULAR_FILE,
    DIRECTORY,
    DEVICE,
    FIFO,
    SYMBOLIC_LINK,
    SOCKET
};

struct file_info {
    long long size;
    bool      is_dir; // left for compatibility reasons
    file_type type;
    
    time_t    modification_time;
    time_t    access_time;
};

typedef std::vector<std::string> file_name_list;

struct directory_entry {
    tstring   name;
    
    file_info info;
};

class lister: public generator_impl<lister, directory_entry> {
public:
    lister(tstring const& path, bool need_stat = false);    
    ~lister();

    bool fetch_next(directory_entry&);
    
private:
    struct internal_data;
    std::auto_ptr<internal_data> data_;
};

struct file_list_visitor {
    virtual void accept(tstring const& name) =0;
    
    virtual ~file_list_visitor(); 
};

void list_files(tstring const& path, file_list_visitor& visitor);
void list_files(tstring const& path, std::vector<std::string>& result);
inline std::vector<std::string> list_files(const char* path) {
    std::vector<std::string> r;
    list_files(path, r);
    return r;
}



file_info stat(tstring const& name);
bool exists(tstring const& name);
bool is_file(tstring const& name);
bool is_dir(tstring const& name);

void cd(tstring const& dirname);
std::string pwd();

void mkdir(tstring const& name, bool create_parents = true);

void copy(tstring const& src, tstring const& dest);
void mv(tstring const& src, tstring const& dest);

void recursive_copy(tstring const& src, tstring const& dest);

void rm(tstring const& name);
void rmdir(tstring const& name);
void recursive_rm(tstring const& src);

// implemented only in POSIX
void         symlink(tstring const& target, tstring const& path);
std::string  readlink(tstring const& path);
std::string  realpath(tstring const& path);

struct walker 
{
    /** Throw this from accept to stop walk. */
    struct stop { };
    
    /** 
        Accept element.
    
        @return callback should return if it's interested in walking
                through children of current element 
        throw throws walker::stop to forcifully stop walking
    */
    virtual bool accept(tstring const& name, tstring const& parent, bool is_dir) = 0;
    
    virtual ~walker();
};

/** Walk through filesystem hierarchy.

*/
void walk(tstring const& start, walker& w);

} } // end namespace tinfra::fs


#endif // tinfra_fs_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
