//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

//
// posix/thread.h
//   pthread based implementation of threads
//


#ifndef __tinfra_posix_thread_h__
#define __tinfra_posix_thread_h__

#include <tinfra/platform.h>
#include <tinfra/runner.h>

#include <vector>
#include <pthread.h>

namespace tinfra {

class Mutex {
    pthread_mutex_t mutex_;
public:
    
    typedef pthread_mutex_t handle_type;

    Mutex() { ::pthread_mutex_init(&mutex_, 0); }
    ~Mutex() { ::pthread_mutex_destroy(&mutex_); }

    void lock() { 
        ::pthread_mutex_lock(&mutex_); 
    }
    void unlock() { 
        ::pthread_mutex_unlock(&mutex_); 
    }
    pthread_mutex_t* get_native() { return &mutex_; }
};

class Condition {
    pthread_cond_t cond_;
public:
    typedef pthread_cond_t handle_type;

    Condition()  { ::pthread_cond_init(&cond_, 0); }
    ~Condition() { ::pthread_cond_destroy(&cond_); }
    
    void signal()    { 
        ::pthread_cond_signal(&cond_); 
    }
    void broadcast() {
        ::pthread_cond_broadcast(&cond_); 
    }
    void wait(pthread_mutex_t* mutex) { 
        ::pthread_cond_wait(&cond_, mutex );
    }
    void wait(Mutex& mutex) { 
        ::pthread_cond_wait(&cond_, mutex.get_native() );
    }
};

class Thread {
    pthread_t thread_;
public:
    explicit Thread(pthread_t thread): thread_(thread) {}
    static Thread current() { return Thread(::pthread_self()); }
    static void sleep(long milliseconds);
    typedef void* (thread_entry)(void*);

    static Thread start( Runnable& runnable);
    /// Start a detached thread
    /// runnable will be deleted before thread end
    static Thread start_detached( Runnable* runnable);    
    
    static Thread start( thread_entry entry, void* param );
    static Thread start_detached( thread_entry entry, void* param );
    
    void* join();
	size_t to_number() const;
};

} // end namespace tinfra


#endif // #ifndef __tinfra_posix_thread_h__
