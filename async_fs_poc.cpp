
#include "tinfra/async_fs.h"


#include "tinfra/shared_ptr.h"
#include "tinfra/symbol.h"
#include "tinfra/mutex.h"
#include "tinfra/guard.h"
#include "tinfra/thread_runner.h"

#include <map>
#include <string>
#include <iostream>

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

namespace afs = tinfra::async_fs;

///
/// default_async_fs
///

class default_async_fs: public afs::service {
    tinfra::static_thread_pool_runner runner;
public:
    default_async_fs(): 
        runner(10),
        last_task_id(0)
    {
    }
    
    struct task_record {
        tinfra::mutex     lock;     //! task_record access lock
        
        default_async_fs* parent;   //! pointer to async_fs object owning this task
        afs::task_id      id;       //! id of task
        bool              cancelled;//! whether task is cancelled fro outside
        bool              notified; //! whether task was notified at least once
    };
    
    struct read_file_job {
        task_record*                 job_task_record;
        
        std::string                  name;        
        afs::callback<afs::content_completion> listener;
        
        // job entry point:
        void operator()()
        {
            std::ostringstream tmp;
            // todo read file
            
            afs::content_completion result;
            result.task = job_task_record->id;
            result.error.error_code = 0;
            result.error.error_message = "";
            result.is_last = true;
            
            result.data = tmp.str();
            result.total_size = result.data.size();
            
            {
                tinfra::guard g(job_task_record->lock); // section locked with task_record::lock
                
                if( job_task_record->cancelled ) {
                    // TRACE that cancellation was seen
                } else {
                    // TRACE that callback is being invoked
                    listener(result);
                    job_task_record->notified = true;
                }                
            }
            
            
            job_task_record->parent->completed(job_task_record->id);
        }
    };
    
    virtual afs::task_id   read_file(std::string const& name, tinfra::callback<afs::content_completion> listener)
    {
        task_record* new_task_record = create_new_task_record();
        read_file_job read_file_job = { new_task_record, name, listener };
            
        runner(read_file_job);
        
        return new_task_record->id;
    }
    
    virtual afs::task_id   list_folder(std::string const& name, tinfra::callback<afs::listing_completion> listener)
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
    int last_task_id;
    tinfra::thread::mutex lock;
    std::map<afs::task_id, task_record*> task_records;

    void cancel(afs::task_id cancelled_task_id)
    {
        task_record* cancelled_task_record = get_task_record(cancelled_task_id);
        assert( cancelled_task_record != 0 );
        {
            tinfra::guard g(cancelled_task_record->lock); // section locked with task_record::lock
            cancelled_task_record->cancelled = true;
        }
    }
    
    void completed(afs::task_id task_to_cancel)
    {
        tinfra::guard g(this->lock);
        
        task_record* cancelled_task_record = get_task_record_unlocked(task_to_cancel);
        assert( cancelled_task_record != 0 );
        assert( task_to_cancel == cancelled_task_record->id );
        
        task_records.erase(task_to_cancel);
        
        delete cancelled_task_record;
    }
    
    task_record* get_task_record(afs::task_id id)
    {
        tinfra::guard g(this->lock);
        
        return get_task_record_unlocked(id);
    }
    
    task_record* get_task_record_unlocked(afs::task_id id)
    {
        std::map<afs::task_id, task_record*>::const_iterator i = task_records.find(id);
        
        if( i != task_records.end() )
            return i->second;
        else
            return 0;
    }
    
    /// creates and stores new task record (with unique id)
    task_record* create_new_task_record()
    {
        std::auto_ptr<task_record> ptr(new task_record());
        ptr->parent    = this;
        ptr->cancelled = false;
        ptr->notified  = false;
        {
            tinfra::guard g(this->lock);

            afs::task_id result_id = ++last_task_id;
            ptr->id        = result_id;
            task_records[result_id]  = ptr.get();
        }
    
        return ptr.release();
    }
};


int main(int argc, char** argv)
{
	return 0;
}



