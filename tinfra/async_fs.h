#ifndef tinfra_async_fs_h_included
#define tinfra_async_fs_h_included

#include <tinfra/fs.h>       // for tinfra::fs::file_info
#include <tinfra/callback.h> // for tinfra::callback

#include <string>
#include <vector>


namespace tinfra {
namespace async_fs {

typedef int task_id;

struct error_condition {
    int         error_code;
    std::string error_message;
};

template <typename T>
struct completion: public T {
    task_id          task;
    error_condition  error;
    bool             is_last;
};

struct content {
    std::string data;
    size_t      total_size;
};

struct file_entry {
    std::string name;

    tinfra::fs::file_info info;
};

typedef std::vector<file_entry> file_entry_list;

struct listing {
    file_entry_list  entries;
};


typedef completion<content> content_completion;
typedef completion<listing> listing_completion;

using tinfra::callback;

struct service {
    virtual task_id   read_file(
        std::string const& name,
        callback<content_completion> listener) = 0;
    
    virtual task_id   list_folder(
        std::string const& name, 
        callback<listing_completion> listener) = 0;

    virtual void cancel(task_id task);

    virtual void process() = 0;

    virtual ~service() {}
};

} } // end namsepace tinfra::async_fs

#endif // tinfra_async_fs_h_included

