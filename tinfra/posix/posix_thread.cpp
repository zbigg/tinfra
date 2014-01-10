//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

//
// posix/thread.cpp
//   pthread based implementation of threads
//

#include "../platform.h"
#include "../config-priv.h"


#ifdef TINFRA_POSIX

#include "tinfra/thread.h"
#include "tinfra/fmt.h"
#include "tinfra/runtime.h"
#include "tinfra/logger.h"

#include <memory>
#include <stdexcept>

#include <pthread.h>

#ifdef HAVE_NANOSLEEP
#include <time.h>
#include <cerrno>
#elif defined HAVE_USLEEP
#include <unistd.h>
#endif

namespace tinfra {
namespace thread {

static void thread_error(const char* message, int rc)
{
    throw std::runtime_error(fmt("failed to %s: %i") % message % rc);
}

struct thread_entry_param {
    void*             (* entry)(void*);
    void*                param;
};

static void* thread_master_fun(void* param)
{
    try {
        std::auto_ptr<thread_entry_param> p2((thread_entry_param*)param);
        return p2->entry(p2->param);
    } catch(std::exception& e) {
        TINFRA_LOG_ERROR( fmt("thread %i failed with uncaught exception: %s\n") % thread::current().to_number() % e.what() );
        return 0;
    }
}

thread thread::start(thread_entry entry, void* param )
{
    pthread_t tid;
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    std::auto_ptr<thread_entry_param> param2(new thread_entry_param());
    param2->entry = entry;
    param2->param = param;
    int rc = pthread_create(&tid, &attr, thread_master_fun, (void *)param2.get());
    param2.release();
    pthread_attr_destroy(&attr);
    
    if( rc != 0 ) 
        thread_error("start thread", rc);
    
    return thread(tid);
}

thread thread::start_detached( thread::thread_entry entry, void* param )
{
    pthread_t tid;
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    std::auto_ptr<thread_entry_param> param2(new thread_entry_param());
    param2->entry = entry;
    param2->param = param;
    int rc = pthread_create(&tid, &attr, thread_master_fun, (void *)param2.get());
    param2.release();
    
    pthread_attr_destroy(&attr);
    
    if( rc != 0 ) 
        thread_error("start thread", rc);
    
    return thread(tid);
}

static void* runnable_entry(void* param)
{
    std::auto_ptr<runnable> param_holder(reinterpret_cast<runnable*>(param));
    
    runnable_base& job= param_holder->get();
    
    job();
    
    // runnable_ptr passed as param by thread::start or thread::start_detached
    // is destroyed here in param_holder destruction
    return 0;
}

thread thread::start(runnable job)
{
    std::auto_ptr<runnable_ptr> param_holder( new runnable_ptr(job) );
    thread t = start(runnable_entry, param_holder.get());
    
    // this is executed only and only is start doesn't
    // throw and it means that runnable_entry will
    // remove *param_holder
    param_holder.release();
    return t;
}

void thread::start_detached(runnable job)
{   
    std::auto_ptr<runnable_ptr> param_holder( new runnable_ptr(job) );
    start_detached(runnable_entry, param_holder.get());
    
    // this is executed only and only is start doesn't
    // throw and it means that runnable_entry will
    // remove *param_holder
    param_holder.release();
}

void* thread::join()
{
    void* retvalue;
    int rc = ::pthread_join(thread_, &retvalue);
    if( rc != 0 ) thread_error("join thread", rc);
    return retvalue;
}

void thread::sleep(long milliseconds)
{
#if defined(WIN32)
    ::Sleep(milliseconds);
#elif defined(HAVE_NANOSLEEP)
    // nanosleep accepts nanoseconds
    timespec req;
    timespec rem;
    req.tv_sec  = milliseconds / 1000;
    req.tv_nsec = (milliseconds % 1000) * 1000000;
    while( true ) {
        int r = ::nanosleep(&req, &rem);
        if( r < 0 ) {
            if( errno == EINTR ) {
                tinfra::test_interrupt();
            }
            req = rem;
            continue;
        }
        break;
    }
#elif defined(HAVE_USLEEP)
    // usleep accepts microseconds
    int r = ::usleep(milliseconds/1000);
    if( r == -1 && errno == EINTR )
        tinfra::test_interrupt();
#else
    thread_error("sleep not implemented on this platform",0);
#endif
}

size_t thread::to_number() const
{
	size_t* a = (size_t*)&thread_;
	return *a;
}

} } // end namespace tinfra::thread

#endif // TINFRA_POSIX

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
