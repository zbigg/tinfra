#include <string>
#include <iostream>

#include "tinfra/shared_ptr.h"
#include "tinfra/callback.h"

#include "tinfra/symbol.h"
#include "tinfra/mutex.h"
#include "tinfra/guard.h"

#include "callfwd.h"

// general idea for ASYNC file system access

// create async-fs wrapper over generic synchronous FS:
// when retrieving something one must have
// async_fs_client instance.
// async_fs_client manages lifecycle of FS requests
//  * when it's deleted, all outstanding requests are cancelled
//    so after completion no dangling pointers are left in async_fs dispatcher
//  * async_fs_client has vfs similar api, but receives function objects that
//    receive results
//  * example:

//    async_fs_client.read_file(tstring const& path, kurna_mac_bind(&MyControl::fileLoadCompleted, *this, _1))

//    async_fs_client.list_folder(tstring const& path, tinfra::callback(*this, &MyControl::fileLoadCompleted)

typedef int async_fs_task_id;
struct error_condition {
    int errno;
    std::string error_message;
};

template <typename T>
struct completion: public T {
    async_fs_task_id id;
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

typedef std::vector<listing_entry> file_entry_list;

struct listing {
    file_entry_list  entries;
};


typedef completion<content> content_completion;
typedef completion<listing> listing_completion;

struct async_fs {
    virtual async_fs_task_id   read_file(string const& name, callback<content_completion> callback) = 0;
    virtual async_fs_task_id   list_folder(string const& name, callback<listing_completion> callback) = 0;

    virtual void cancel(async_fs_task_id task_id);

    virtual void process() = 0;

    virtual ~async_fs() {}
};

struct async_fs_client {
    virtual async_fs_task_id   read_file(string const& name, callback<content_completion> callback) = 0;
    virtual async_fs_task_id   list_folder(string const& name, callback<listing_completion> callback) = 0;

    virtual void cancel(async_fs_task_id task_id) = 0;
    
    virtual async_fs_client() {}
};

///
/// default_async_fs
///

class default_async_fs: public async_fs {
    static_thread_pool_runner runner;
public:
    default_async_fs(): 
        runner(10)
    {
    }
    
    struct task_record {
        tinfra::mutex     lock;     //! task_record access lock
        
        default_async_fs* parent;   //! pointer to async_fs object owning this task
        async_fs_task_id  id;       //! id of task
        bool              canceled; //! whether task is cancelled fro outside
        bool              notified; //! whether task was notified at least once
    };
    
    struct read_file_job {
        task_record*                 record;
        
        std::string                  name;        
        callback<content_completion> callback;
        
        // job entry point:
        void operator()()
        {
            std::ostringstream tmp;
            // todo read file
            
            content_completion result;
            result.id = task_id;
            result.error.errno = 0;
            result.error.error_message = "";
            result.content = tmp.str();
            result.is_last = true;
            result.total_size = result.content.size();
            
            {
                tinfra::guard g(my_task_record->lock); // section locked with task_record::lock
                
                if( my_task_record->cancelled ) {
                    // TRACE that cancellation was seen
                } else {
                    // TRACE that callback is being invoked
                    callback(result);
                    my_task_record->notified = true;
                }                
            }
            
            
            parent.completed(my_task_record->id);
        }
    };
    
    virtual async_fs_task_id   read_file(string const& name, callback<content_completion> callback)
    {
        task_record* new_task_record = create_new_task_record();
        read_file_job read_file_job = { new_task_record, name, callback };
            
        thread_pool(read_file_job);
        
        return new_task_record->id;
    }
    
    virtual async_fs_task_id   list_folder(string const& name, callback<listing_completion> callback)
    {
        assert(0);
        /*
        task_record* new_task_record = create_new_task_record();
        read_file_job listing_job = { new_task_record, name, callback };
            
        runner(listing_job);
        
        return new_task_record->id;
        */
    }
    
private:
    tinfra::thread::mutex lock;
    std::map<async_fs_task_id, task_record*> task_records;

    void cancel(async_fs_task_id cancelled_task_id)
    {
        task_record* cancelled_task_record = get_task_record(cancelled_task_id);
        assert( cancelled_task_record != 0 );
        {
            tinfra::guard g(cancelled_task_record->lock); // section locked with task_record::lock
            cancelled_task_record->cancelled = true;
        }
    }
    
    void completed(async_fs_task_id id)
    {
        tinfra::guard g(this->lock);
        
        task_record* cancelled_task_record = get_task_record_unlocked(id);
        assert( cancelled_task_record != 0 );
        
        task_records.erase(cancelled_task_id->id);
        
        delete cancelled_task_record;
    }
    
    task_record* get_task_record(async_fs_task_id id)
    {
        tinfra::guard g(this->lock);
        
        return get_task_record_unlocked(id);
    }
    
    task_record* get_task_record_unlocked(async_fs_task_id id)
    {
        std::map<async_fs_task_id, task_record*>::const_iterator i = task_records.find(id);
        
        if( i != task_records.end() )
            return i->second;
        else
            return 0;
    }
    
    /// creates and stores new task record (with unique id)
    task_record* create_new_task_record(async_fs_task_id id)
    {
        tinfra::guard g(this->lock);
        std::auto_ptr<task_record> ptr(new task_record());
        
        async_fs_task_id result_id = ++last_task_id;
        
        ptr->parent    = this;
        ptr->id        = result_id;
        ptr->cancelled = false;
        ptr->notified  = false;
        
        task_records[result_id]  = ptr.get();
    
        return ptr.release();
    }
};

struct default_async_fs_client: public async_fs_client {
    virtual async_fs_task_id   read_file(string const& name, callback<content_completion> callback)
    {
    }
    virtual async_fs_task_id   list_folder(string const& name, callback<listing_completion> callback)
    {
    }

    virtual void cancel(async_fs_task_id task_id) = 0;
    
    virtual async_fs_client() {}
};


int main(int argc, char** argv)
{
	async_fs fs;
	writer wr;
	callfwd::call_sender<writer> target(wr);
	fs.read_file("test", make_callback<read_file_completion>(target));
	fs.process();
	return 0;
}



