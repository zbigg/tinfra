//
// Copyright (C) 2009 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_runner_h_included_
#define tinfra_runner_h_included_

#include <tinfra/shared_ptr.h>

#include <memory>
#include <list>

#include "value_guard.h"

namespace tinfra {

class runnable_base {
public:
    virtual ~runnable_base();
    void operator()() {
        do_run();
    }
    
private:
    virtual void do_run() = 0;
};

template <typename T>
class runnable_adapter: public runnable_base {
public:
    runnable_adapter(T delegate):
        delegate_(delegate)
    {}
        
private:
    virtual void do_run()
    {
        delegate_();
    }
    T delegate_;
};

template <typename T>
class runnable_ref_adapter: public runnable_base {
public:
    runnable_ref_adapter(T& delegate):
        delegate_(delegate)
    {}
        
private:
    virtual void do_run()
    {
        delegate_();
    }
    T& delegate_;
};

template <typename T>
runnable_ref_adapter<T> runnable_ref(T& r)
{
    return runnable_ref_adapter<T>(r);
}

class runnable {
    shared_ptr<runnable_base> ptr_;
    
public:
    runnable(shared_ptr<runnable_base> other_):
        ptr_(other_)
    {}
        
    template <typename T>
    runnable(runnable_ref_adapter<T> const& ref_copy):
        ptr_(new runnable_ref_adapter<T>(ref_copy))
    {}
        
        
    template <typename T>
    runnable(T const& p):
        ptr_(new runnable_adapter<T>(p))
    {
    }
    
    runnable_base& get() { 
        assert( ptr_.get() != 0 );
        return *ptr_;
    }
    
    bool operator ==(runnable const& other)
    {
        return ptr_.get() == other.ptr_.get();
    }
    
    bool operator !=(runnable const& other)
    {
        return ptr_.get() != other.ptr_.get();
    }
    
    static runnable EMPTY_RUNNABLE;
};

// TODO: remove all runnable_ptr references
typedef runnable runnable_ptr;

class runner {
public:
    virtual ~runner();
    
    void operator()(runnable p)
    {
        do_run(p);
    }
    
private:
    virtual void do_run(runnable_ptr const&) = 0;
};

class sequential_runner: public runner {
public:
    sequential_runner():
        currently_executing_(false)
    {
    }
    ~sequential_runner();
private:
    void do_run(runnable_ptr const& p)
    {
        enqueue(p);
        if( currently_executing_ ) {
            return;
        }
        
        while( !queue_.empty() ) {
            TINFRA_TRACE_MSG("sequential_runner::do_run: starting job");
            runnable_ptr current_ptr = queue_.front();
            queue_.pop_front();
            
            runnable_base& current_job = current_ptr.get();
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




} // end namespace tinfra

#endif // tinfra_runner_h_included_

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

