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
    size_t               stack_size;
};

void initialize_stack_params(thread_entry_param* tp);

static void* thread_master_fun(void* param)
{
    std::auto_ptr<thread_entry_param> p2((thread_entry_param*)param);
    
    initialize_stack_params(p2.get());
    
    try {

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
    
    std::auto_ptr<thread_entry_param> param2(new thread_entry_param());
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    pthread_attr_getstacksize(&attr, &param2->stack_size);
    
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
    
    pthread_attr_getstacksize(&attr, &param2->stack_size);
    
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
#if defined(WIN32)
    ::Sleep(milliseconds);
#elif defined(HAVE_NANOSLEEP)
    // nanosleep accepts nanoseconds
    timespec req,rem;
    req.tv_sec  = milliseconds / 1000000000;
    rem.tv_nsec = milliseconds % 1000000000;
    ::nanosleep(&req, &rem);
#elif defined(HAVE_USLEEP)
    // usleep accepts microseconds
    ::usleep(milliseconds/1000);
#else
    thread_error("sleep not implemented on this platform",0);
#endif
}

size_t Thread::to_number() const
{
	size_t* a = (size_t*)&thread_;
	return *a;
}

// 
// stack related things implementation
//

__thread const void* tg_stack_bottom = 0;
__thread size_t      tg_stack_size = 0;

void guess_stack_parameters();
const void* get_stack_bottom()
{
    if( tg_stack_bottom == 0  ) {
        guess_stack_parameters();
    }
    return tg_stack_bottom;
}

void initialize_stack_params(thread_entry_param* tp)
{
#ifdef linux
    guess_stack_parameters();
#else
    if( tp->stack_size == 0 ) {
        guess_stack_parameters();
    } else {
        char a;
        tg_stack_bottom = a - tp->stack_size;
        tg_stack_size   = tp->stack_size;
    }
#endif
}

void guess_stack_parameters()
{
#ifdef linux
    {
        pthread_attr_t attr;
        int res = pthread_getattr_np(pthread_self(), &attr);
        if (res != 0) {
            throw std::runtime_error("pthread_getattr_np");
        }
        
        char* stack_bottom;
        size_t stack_bytes;
        res = pthread_attr_getstack(&attr, (void **) &stack_bottom, &stack_bytes);
        if (res != 0) {
            pthread_attr_destroy(&attr);
            throw std::runtime_error("pthread_getattr_np");
        }
        tg_stack_bottom = stack_bottom;
        tg_stack_size   = stack_bytes;
    }
#else
    char* stack_current;
    size_t stack_size;
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        
        pthread_attr_getstacksize(&attr, stack_size);
    }
    
    tg_stack_size   = stack_size;
    tg_stack_bottom = &stack_current - stack_size;
#endif
}
}

