//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

//
// win32/thread.cpp
//   pthread based implementation of threads
//   STILL NOT READY

#include "tinfra/thread.h"

#include <iostream>

#include <windows.h>
#include <process.h>
#include <limits.h>

#include <memory>

#include "tinfra/win32.h"
#include "tinfra/fmt.h"

#include "tinfra/trace.h"



namespace tinfra {
namespace thread {

TINFRA_MODULE_TRACER(tinfra_thread);

static void thread_error(const char* message, unsigned int rc)
{
    throw generic_exception(fmt("tinfra::thread error: %s :%s(%i)") % message % win32::get_error_string(rc) % rc );
}

//
// mutex implementation
//

mutex::mutex() {
    ::InitializeCriticalSection(&mutex_);
}

mutex::~mutex() {
    ::DeleteCriticalSection(&mutex_);
}

void mutex::lock() {
    ::EnterCriticalSection(&mutex_);
}

void mutex::unlock() {
    ::LeaveCriticalSection(&mutex_);
}

// 
// condition implementation
//
TINFRA_PUBLIC_TRACER(tinfra_thread_condition);
condition::condition()
:   current_generation(0),
    signal_count(0),
    signal_sem(0),
    signal_generation(0),
    waiters_count(0),
    was_broadcast(false),
    broadcast_ended(0)
{
    TINFRA_USE_TRACER(tinfra_thread_condition);
    TINFRA_CALL_TRACE();

    signal_sem = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
    if( signal_sem == 0 ) {
        TINFRA_LOG_WIN32_ERROR("create semaphore failed", GetLastError());
        thread_error("create semaphore", GetLastError());
    }
    broadcast_ended = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
    if( broadcast_ended == 0 ) {
        TINFRA_LOG_WIN32_ERROR("create semaphore failed", GetLastError());
        thread_error("create semaphore", GetLastError());
    }

}

condition::~condition() {
    TINFRA_USE_TRACER(tinfra_thread_condition);
    TINFRA_CALL_TRACE();

    CloseHandle(signal_sem);
    CloseHandle(broadcast_ended);
}

void condition::signal() {
    TINFRA_USE_TRACER(tinfra_thread_condition);
    TINFRA_CALL_TRACE();

    guard interal_guard(internal_lock);
    if( waiters_count == 0 )
        return;
    signal_count  +=1;
    waiters_count -= 1;
    was_broadcast = false;
    signal_generation = current_generation;
    ReleaseSemaphore(signal_sem, 1, 0);
}

void condition::broadcast() {
    TINFRA_USE_TRACER(tinfra_thread_condition);
    TINFRA_CALL_TRACE();
    {
        guard interal_guard(internal_lock);
        if( waiters_count == 0 )
            return;
        signal_count = waiters_count;
        waiters_count = 0;
        was_broadcast = true;
        signal_generation = current_generation;
        ReleaseSemaphore(signal_sem, signal_count, 0);
    }
    WaitForSingleObject(broadcast_ended, INFINITE);
}

void condition::wait(mutex& external_lock) {
    TINFRA_USE_TRACER(tinfra_thread_condition);
    TINFRA_CALL_TRACE();

    unsigned long waiter_generation;
    {
        guard interal_guard(internal_lock);
        waiters_count += 1;
        waiter_generation = ++current_generation;
    }
    external_lock.unlock();

    while( true ) {
        WaitForSingleObject(signal_sem, INFINITE);

        guard interal_guard(internal_lock);
        if( signal_generation < waiter_generation ) {
            ReleaseSemaphore(signal_sem, 1, 0);
            continue;
        }
        signal_count -= 1;
        if( signal_count == 0 && was_broadcast ) {
            ReleaseSemaphore(broadcast_ended, 1, 0);
        }
        break;
    }
    external_lock.lock();
}

//
// thread implementation
//
struct thread_entry_param {
    void*             (* entry)(void*);
    void*                param;
};

static unsigned __stdcall thread_master_fun(void* raw_params)
{
    size_t tid = thread::current().to_number();

    TINFRA_TRACE_MSG(fmt("entered thread tid=%i") % tid);
    unsigned result;
    try {
        std::auto_ptr<thread_entry_param> params((thread_entry_param*)raw_params);
        params->entry(params->param);
        // TODO dupa!
        result = 0;
    } catch(std::exception& e) {
        TINFRA_LOG_ERROR(fmt("thread %i failed with uncaught exception: %s\n") % tid % e.what());
        result = ~0;
    }
    TINFRA_TRACE_MSG(fmt("thread exited tid=%i") % tid );
    ::_endthreadex(result); // never returns
    return result; // just to satisfy compiler
}

thread thread::start(thread_entry entry, void* param )
{    
    std::auto_ptr<thread_entry_param> te_params(new thread_entry_param());
    te_params->entry = entry;
    te_params->param = param;
    unsigned thread_id;
    uintptr_t thread_handle = ::_beginthreadex(
                           NULL, // no security desc
                           0,        // same stack as parent
                           thread_master_fun, 
                           te_params.get(),
                           0,  // start immediately
                           &thread_id);
    
    if( thread_handle == -1 ) // TODO: check this condition
        thread_error("_beginthreadex failed", ::GetLastError());
    te_params.release();
    
    return thread((HANDLE)thread_handle, thread_id);
}

void thread::start_detached( thread::thread_entry entry, void* param )
{
    // no difference detached/joinable on win32
    thread t = start(entry, param);
    ::CloseHandle( t.native_handle() );
}

thread thread::current()
{
    return thread(NULL, GetCurrentThreadId());
    /*
    HANDLE handle = ::OpenThread(THREAD_QUERY_INFORMATION, FALSE, GetCurrentThreadId());
    if( handle == 0 ) {
        TINFRA_LOG_WIN32_ERROR("OpenThread failed", GetLastError());
        thread_error("OpenThread failed", GetLastError());
    }
    thread result(handle);

    CloseHandle(handle);
    return result;
    */
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

thread::thread(handle_type thread_handle, DWORD thread_id)
    :   
    thread_handle_(thread_handle),
    thread_id_(thread_id)
{
}

thread::~thread()
{
}

thread thread::start( Runnable& runnable )
{
    return start(runnable_entry, (void*) &runnable);
}

void thread::start_detached( Runnable* runnable)
{   
    start_detached(runnable_entry_delete, (void*) &runnable);    
}

void* thread::join()
{
    if( thread_handle_ == NULL ) {
        throw std::logic_error("trying to join already joined or detached thread");
    }
    
    size_t tid = to_number();
    TINFRA_TRACE_MSG(fmt("thread joining ... tid=%i") % tid);
    DWORD gla;
    
    int rc = ::WaitForSingleObject( thread_handle_, INFINITE );
    if( rc != WAIT_OBJECT_0 ) {
        TINFRA_LOG_WIN32_ERROR("WaitForSingleObject failed", GetLastError());
        gla = ::GetLastError();
    }
    
    DWORD retvalue;
    BOOL rc2 = ::GetExitCodeThread( thread_handle_, &retvalue );
    if( !rc2 ) {
        TINFRA_LOG_WIN32_ERROR("GetExitCodeThread failed", GetLastError());
        // silent error, can't ger exit_code
        retvalue = 0;
    }
    ::CloseHandle(thread_handle_);
    thread_handle_ = 0;

    if( rc != WAIT_OBJECT_0 )
        thread_error("unable to join thread", gla);
    
    TINFRA_TRACE_MSG(fmt("thread joined tid=%i, result %i") % tid % retvalue);
    return (void*)retvalue;
}

void thread::sleep(long milliseconds)
{
    ::Sleep(milliseconds);
}

size_t thread::to_number() const
{    
    return static_cast<size_t>(thread_id_);
}

} } // end namespace tinfra::thread


