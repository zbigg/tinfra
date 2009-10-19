//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_thread_runner_h_included
#define tinfra_thread_runner_h_included

#include <tinfra/thread.h>
#include <tinfra/queue.h>

#include <memory>
#include <list>

#include "value_guard.h"
#include "runner.h"

namespace tinfra {

class thread_runner: public runner {
    void do_run(runnable_ptr const& p)
    {
        std::auto_ptr<runnable_ptr> holder(new runnable_ptr(p));
        tinfra::thread::thread::start_detached(&thread_runner::static_runnable_invoker, (void*)holder.get());
        
        // NOTE about ownership of p and holder
        // in case of exception, this release
        // will be skipped and *holder will be deleted
        // in case of success, it will be released
        // static_runnable_invoker will delete *holder
        holder.release();
    }

    static void* static_runnable_invoker(void* p) {
        std::auto_ptr<runnable_ptr> holder( static_cast<runnable_ptr*>(p));
        
        runnable_base& current_job = holder->get();
        
        current_job();
        
        // current_job is released
        holder.reset();
        return 0;
    }
};

class static_thread_pool_runner: public runner {
    int thread_count_;
    
    tinfra::thread::thread_set  threads_;
    tinfra::queue<runnable_ptr> queue_;
public:
    static_thread_pool_runner(int thread_count):
        thread_count_(thread_count)
    {
        for(int i = 0; i < thread_count_; ++i ) {
            threads_.start(&static_thread_pool_runner::worker_thread_func, &queue_);
        }
    }
    
    ~static_thread_pool_runner()
    {
        for(int i = 0; i < thread_count_; ++i ) {
            queue_.put( runnable::EMPTY_RUNNABLE );
        }
        threads_.join();
    }
private:
    void do_run(runnable_ptr const& p)
    {
        queue_.put(p);
    }
    
    static void* worker_thread_func(void* p) {
        tinfra::queue<runnable_ptr>& queue = *(static_cast<tinfra::queue<runnable_ptr>*>(p));
        
        while( true ) {
            runnable_ptr current_ptr = queue.get();
            
            if( current_ptr == runnable::EMPTY_RUNNABLE ) {
                break;
            }
            
            runnable_base& current_job = current_ptr.get();
            
            current_job();
            // current_job should be destroyed now
        }
        return 0;
    }
};

} // end namespace tinfra

#endif // tinfra_thread_runner_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

