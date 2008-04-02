//
// win32/thread.cpp
//   pthread based implementation of threads
//   STILL NOT READY

#include "tinfra/thread.h"

#include <iostream>

#define _WIN32_WINNT 0x10000
#include <windows.h>
#include <process.h>

#include "tinfra/win32.h"
#include "tinfra/fmt.h"

namespace tinfra {

static void thread_error(const char* message, unsigned int rc)
{
    throw generic_exception(fmt("tinfra::thread error: %s :%s(%i)") % message % win32::get_error_string(rc) % rc );
}

//
// Mutex implementation
//

Mutex::Mutex() {
    ::InitializeCriticalSection(&mutex_);
}

Mutex::~Mutex() {
    ::DeleteCriticalSection(&mutex_);
}

void Mutex::lock() {
    ::EnterCriticalSection(&mutex_);
}

void Mutex::unlock() {
    ::LeaveCriticalSection(&mutex_);
}

// 
// Condition implementation
//

Condition::Condition() {
    ::InitializeConditionVariable(&cond_);
}

Condition::~Condition() {
}

void Condition::signal() {
    ::WakeConditionVariable(&cond_);
}

void Condition::broadcast() {
    ::WakeAllConditionVariable(&cond_);
}

void Condition::wait(void* mutex) {
    CRITICAL_SECTION* cs = (CRITICAL_SECTION*)mutex;
    
    BOOL r = ::SleepConditionVariableCS(&cond_, cs, INFINITE);
    if( r == 0 )
        thread_error("wait for condition", ::GetLastError());    
}

void Condition::wait(Mutex& mutex) {
    wait( mutex.get_native() );
}

//
// Thread implementation
//
struct thread_entry_param {
    void*             (* entry)(void*);
    void*                param;
};

static unsigned __stdcall thread_master_fun(void* raw_params)
{
    unsigned result;
    try {
        std::auto_ptr<thread_entry_param> params((thread_entry_param*)raw_params);
        params->entry(params->param);
        // TODO dupa!
        result = 0;
    } catch(std::exception& e) {
        std::cerr << fmt("thread %i failed with uncaught exception: %s\n") % Thread::current().to_number() % e.what();
        result = 0;
    }
    ::_endthreadex(result);
}

Thread Thread::start(thread_entry entry, void* param )
{    
    std::auto_ptr<thread_entry_param> te_params(new thread_entry_param());
    te_params->entry = entry;
    te_params->param = param;
    unsigned thread_id;
    uintptr_t ihandle = ::_beginthreadex(NULL, // no security desc
                                     0,        // same stack as parent
                                     thread_master_fun, 
                                     te_params.get(),
                                     0,      // start immediately
                                     &thread_id);
    
    if( ihandle == -1L ) 
        thread_error("start thread", ::GetLastError());
    
    te_params.release();
    
    return Thread(thread_id);
}

Thread Thread::start_detached( Thread::thread_entry entry, void* param )
{
    // no difference detached/joinable on win32
    return start(entry, param);
}

Thread Thread::current()
{
    return Thread(GetCurrentThreadId());
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
    HANDLE handle = ::OpenThread(THREAD_ALL_ACCESS, FALSE, thread_id_);
    if( handle == NULL )
        thread_error("join: unable to open thread", ::GetLastError());
    
    
    DWORD gla;
    
    int rc = ::WaitForSingleObject( handle, INFINITE );
    if( rc == 0 ) 
        gla = ::GetLastError();
    
    DWORD retvalue;
    unsigned exit_code = ::GetExitCodeThread( handle, &retvalue );
    
    ::CloseHandle(handle);
    
    if( rc == 0 )
        thread_error("unable to join thread", gla);
    
    return (void*)retvalue;
}

void Thread::sleep(long milliseconds)
{
    ::Sleep(milliseconds);
}

size_t Thread::to_number() const
{    
    return static_cast<size_t>(thread_id_);
}

}

