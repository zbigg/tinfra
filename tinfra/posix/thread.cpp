//
// posix/thread.cpp
//   pthread based implementation of threads
//

#include "tinfra/thread.h"

#include <iostream>

#include <pthread.h>
#include "tinfra/fmt.h"

#ifdef HAVE_NANOSLEEP
#include <time.h>
#elif defined HAVE_USLEEP
#include <unistd.h>
#endif

namespace tinfra {

static void thread_error(const char* message, int rc)
{
    throw generic_exception(fmt("failed to %s: %i") % message % rc);
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
        std::cerr << fmt("thread %i failed with uncaught exception: %s\n") % Thread::current().to_number() % e.what();
        return 0;
    }
}

Thread Thread::start(thread_entry entry, void* param )
{
    pthread_t thread;
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    std::auto_ptr<thread_entry_param> param2(new thread_entry_param());
    param2->entry = entry;
    param2->param = param;
    int rc = pthread_create(&thread, &attr, thread_master_fun, (void *)param2.get());
    param2.release();
    pthread_attr_destroy(&attr);
    
    if( rc != 0 ) 
        thread_error("start thread", rc);
    
    return Thread(thread);
}

Thread Thread::start_detached( Thread::thread_entry entry, void* param )
{
    pthread_t thread;
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    std::auto_ptr<thread_entry_param> param2(new thread_entry_param());
    param2->entry = entry;
    param2->param = param;
    int rc = pthread_create(&thread, &attr, thread_master_fun, (void *)param2.get());
    param2.release();
    
    pthread_attr_destroy(&attr);
    
    if( rc != 0 ) 
        thread_error("start thread", rc);
    
    return Thread(thread);
}

static void* runnable_entry(void* param)
{
    Runnable* runnable = static_cast<Runnable*>(param);
    runnable->run();
    return 0;
}

static void* runnable_entry_delete(void* param)
{
    std::auto_ptr<Runnable> runnable(static_cast<Runnable*>(param));
    runnable->run();
    return 0;
}

Thread Thread::start( Runnable& runnable)
{
    return start(runnable_entry, (void*) &runnable);
}

Thread Thread::start_detached( Runnable* runnable)
{   
    return start_detached(runnable_entry_delete, (void*) &runnable);    
}

void* Thread::join()
{
    void* retvalue;
    int rc = ::pthread_join(thread_, &retvalue);
    if( rc != 0 ) thread_error("join thread", rc);
    return retvalue;
}

void Thread::sleep(long milliseconds)
{
#if defined(HAVE_NANOSLEEP)
    // nanosleep accepts nanoseconds
    timespec req,rem;
    req.tv_sec  = milliseconds / 1000000000;
    rem.tv_nsec = milliseconds % 1000000000;
    ::nanosleep(&req, &rem);
#elif defined(HAVE_USLEEP)
    // usleep accepts microseconds
    ::usleep(milliseconds/1000);
#elif defined WIN32
    ::Sleep(milliseconds);
#else
    thread_error("sleep not implemented on this platform",0);
#endif
}

size_t Thread::to_number() const
{
	size_t* a = (size_t*)&thread_;
	return *a;
}

}

