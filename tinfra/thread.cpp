#include "tinfra/thread.h"

#include <pthread.h>
#include "tinfra/fmt.h"

namespace tinfra {

static void thread_error(const char* message, int rc)
{
    throw generic_exception(fmt("failed to %s: %i") % message % rc);
}

Thread Thread::start( thread_entry entry, void* param )
{
    pthread_t thread;
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int rc = pthread_create(&thread, &attr, entry, (void *)param);
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
    int rc = pthread_create(&thread, &attr, entry, (void *)param);
    pthread_attr_destroy(&attr);
    
    if( rc != 0 ) 
        thread_error("start thread", rc);
    
    return Thread(thread);
}

void* Thread::join()
{
    void* retvalue;
    int rc = ::pthread_join(thread_, &retvalue);
    if( rc != 0 ) thread_error("join thread", rc);
    return retvalue;
}

ThreadSet::~ThreadSet()
{
    join(0);
}

void   ThreadSet::add(Thread t)
{
    threads_.push_back(t);
}

Thread ThreadSet::start(Thread::thread_entry entry, void* param)
{
    Thread t = Thread::start(entry, param);
    threads_.push_back(t);
    return t;
}

void ThreadSet::join(std::vector<void*>* result)
{
    while( threads_.size() > 0 ) {
        std::vector<Thread>::iterator i = threads_.end()-1;
        void* r = i->join();
        threads_.erase(i);
        if( result )
            result->push_back(r);
    }
}

}

