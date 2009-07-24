//
// Copyright (C) 2009 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_thread_pool_h_included_
#define tinfra_thread_pool_h_included_

#include <tinfra/thread.h>
#include <tinfra/shared_ptr.h>
#include <tinfra/queue.h>

#include <memory>
#include <list>

namespace tinfra {

template <typename T>
class value_guard {
    T& ref;
    T  copy;
public:
    value_guard(T& victim):
        ref(victim),
        copy(victim)
    {
    }
    
    ~value_guard() {
        if( !( ref == copy ) ) {
            ref = copy;
        }
    }
};

class runnable {
public:
    virtual ~runnable();
    void operator()() {
        do_run();
    }
    
private:
    virtual void do_run() = 0;
};


typedef shared_ptr<runnable> runnable_ptr;

class runner {
public:
    virtual ~runner();
    
    void operator()(runnable_ptr p)
    {
        do_run(p);
    }
    
    template <typename T>
    void operator()(T const& p) {
        runnable_ptr ptmp(new T(p));
        do_run(ptmp);
    }
private:
    virtual void do_run(runnable_ptr) = 0;
};

class sequential_runner: public runner {
public:
    sequential_runner():
        currently_executing_(false)
    {
    }
    ~sequential_runner();
private:
    void do_run(runnable_ptr p)
    {
        enqueue(p);
        if( currently_executing_ ) {
            return;
        }
        
        while( !queue_.empty() ) {
            TINFRA_TRACE_MSG("sequential_runner::do_run: starting job");
            runnable_ptr current_ptr = queue_.front();
            queue_.pop_front();
            
            runnable& current_job = * ( current_ptr.get() );
            //IMPORTANT(current_job);
            {
                value_guard<bool> guard(currently_executing_);
                currently_executing_ = true;
                current_job();
            }
            // current_job is released
        }
    }
    void enqueue(runnable_ptr p) {
        queue_.push_back(p);
    }
    
    bool currently_executing_;
    std::list<runnable_ptr> queue_;
};


class thread_runner: public runner {
    void do_run(runnable_ptr p)
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
        
        runnable& current_job = * holder->get();
        
        current_job();
        
        // current_job is released
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
            runnable* tmp = 0;
            queue_.put( runnable_ptr(tmp) );
        }
    }
private:
    void do_run(runnable_ptr p)
    {
        queue_.put(p);
    }
    
    static void* worker_thread_func(void* p) {
        tinfra::queue<runnable_ptr>& queue = *(static_cast<tinfra::queue<runnable_ptr>*>(p));
        
        while( true ) {
            runnable_ptr current_ptr = queue.get();
            
            if( current_ptr.get() == 0 ) {
                break;
            }
            
            runnable& current_job = * (current_ptr.get());
            
            current_job();
        }
        return 0;    
    }
};

} // end namespace tinfra

#endif // tinfra_thread_pool_h_included_

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

