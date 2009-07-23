//
// Copyright (C) 2008,2009 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

//
// posix/thread.h
//   pthread based implementation of threads
//


#ifndef tinfra_posix_thread_h_included
#define tinfra_posix_thread_h_included

#include <tinfra/platform.h>
#include <tinfra/runner.h>

#include <vector>
#include <pthread.h>
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

class mutex {
    pthread_mutex_t mutex_;
public:
    
    typedef pthread_mutex_t handle_type;

    mutex() { ::pthread_mutex_init(&mutex_, 0); }
    ~mutex() { ::pthread_mutex_destroy(&mutex_); }

    void lock() { 
        ::pthread_mutex_lock(&mutex_); 
    }
    void unlock() { 
        ::pthread_mutex_unlock(&mutex_); 
    }
    pthread_mutex_t* get_native() { return &mutex_; }
};

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
};

class thread {
    pthread_t thread_;
public:
    explicit thread(pthread_t thread): thread_(thread) {}
    static thread current() { return thread(::pthread_self()); }
    
    static void sleep(long milliseconds);
    typedef void* (thread_entry)(void*);

    static thread start( Runnable& runnable);
    /// Start a detached thread
    /// runnable will be deleted before thread end
    static thread start_detached( Runnable* runnable);    
    
    static thread start( thread_entry entry, void* param );
    static thread start_detached( thread_entry entry, void* param );
    
    void* join();
    size_t to_number() const;
};

} } // end namespace tinfra


#endif // #ifndef tinfra_posix_thread_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

