#include <iostream>

#include "tinfra/generator.h"
//#include "tinfra/fs.h"
#include "tinfra/tstring.h"
#include "tinfra/fmt.h"
#include "tinfra/os_common.h"

#include <dirent.h>
#include <cerrno>

namespace tinfra {
    
struct directory_entry {
    tstring    name;
    //file_info* info; //may be null, TODO: merge in tinfra.vfs  branch
};

class list_files_generator: public generator_impl<list_files_generator, directory_entry> {
public:
    typedef directory_entry value_type;

    list_files_generator(tstring const& path, bool need_stat = false);    
    ~list_files_generator();

    bool fetch_next(directory_entry&);
    
private:
    DIR* dir_;
    bool need_stat_;
    directory_entry entry_;
};

list_files_generator::list_files_generator(tstring const& path, bool need_stat):
    dir_(0),
    need_stat_(need_stat)
{
    string_pool tmppool;
    dir_ = ::opendir(path.c_str(tmppool));
    if( !dir_ ) {
        throw_errno_error(errno, fmt("unable to read dir '%s'") % path);
    }
}

list_files_generator::~list_files_generator()
{
    ::closedir(dir_);
}

bool list_files_generator::fetch_next(directory_entry& de)
{
    dirent* entry;
    while( true ) {
        entry = ::readdir(dir_);
        if( entry == 0 ) {
            return false;
        }
        if(    std::strcmp(entry->d_name,"..") == 0
            || std::strcmp(entry->d_name,".") == 0 ) {
            continue;
        }
        de.name = entry->d_name;
        if( need_stat_ ) {
            // TODO: maybe fill stat
            // fill out de->info
        }
        return true;
    } 
}

std::ostream& operator <<(std::ostream& out, directory_entry const& e)
{
    return out << "file: " << e.name;
}

} // end namespace tinfra


#include <tinfra/cmd.h>

int generator_test_main(int, char** argv)
{
    tinfra::list_files_generator lister(argv[1]);
        
    std::copy(lister.current(), lister.end(), 
        std::ostream_iterator<tinfra::directory_entry>(std::cout,"\n"));
    return 0;
}

TINFRA_MAIN(generator_test_main)
