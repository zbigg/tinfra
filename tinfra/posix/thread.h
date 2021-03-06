//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

//
// posix/thread.h
//   pthread based implementation of threads
//


#ifndef tinfra_posix_thread_h_included
#define tinfra_posix_thread_h_included

#include <tinfra/platform.h>
#include <tinfra/runner.h>
#include <tinfra/fmt.h>

#include <pthread.h> // for many pthread_xxx functions
#include <errno.h> // for ETIMEDOUT

#include <vector>
#include <stdexcept>

//#include <semaphore.h>

namespace tinfra {
namespace thread {
/*
class Semaphore {
    sem_t sem_;        
public:
    Semaphore(int value) { ::sem_init(&sem_, 0, value); }
    ~Semaphore()         { ::sem_destroy(&sem_); }
        
    void post()          { ::sem_post(&sem_); }    
    void wait()          { ::sem_wait(&sem_); }
    
    sem_t* get_native() { return &mutex_; }
};
*/

class condition {
    pthread_cond_t cond_;
public:
    typedef pthread_cond_t handle_type;

    condition()  { ::pthread_cond_init(&cond_, 0); }
    ~condition() { ::pthread_cond_destroy(&cond_); }
    
    void signal()    { 
        ::pthread_cond_signal(&cond_); 
    }
    void broadcast() {
        ::pthread_cond_broadcast(&cond_); 
    }
    void wait(pthread_mutex_t* mutex) { 
        ::pthread_cond_wait(&cond_, mutex );
    }
    void wait(mutex& mutex) { 
        ::pthread_cond_wait(&cond_, mutex.get_native() );
    }
    
    bool timed_wait(mutex& mutex, deadline const& d) {
        struct timespec tspec;
        
        const time_stamp ts = d.get_absolute();
        
        tspec.tv_sec = ts.to_seconds();
        tspec.tv_sec = ( ts.to_milliseconds() % 1000 ) * 1000*1000;
        
        const int r = ::pthread_cond_timedwait(&cond_, mutex.get_native(), &tspec);
        switch( r ) {
        case 0:
            return true;
        case ETIMEDOUT:
            return false;
        default:
            throw std::logic_error(tsprintf("pthread_cond_timedwait failed (r=%i)", r));
                
        }
    }
};

class thread {
    pthread_t thread_;
public:
    explicit thread(): thread_() {}
    explicit thread(pthread_t thread): thread_(thread) {}
    static thread current() { return thread(::pthread_self()); }
    
    static void sleep(long milliseconds);
    typedef void* (*thread_entry)(void*);

    static thread start( runnable job );
    /// Start a detached thread
    /// runnable will be deleted before thread end
    static void   start_detached( runnable job);    
    
    static thread start( thread_entry entry, void* param );
    static thread start_detached( thread_entry entry, void* param );
    
    void* join();
    size_t to_number() const;
};

} } // end namespace tinfra


#endif // #ifndef tinfra_posix_thread_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

