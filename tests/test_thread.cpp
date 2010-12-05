//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/thread.h"
#include "tinfra/guard.h"

#include <sstream>

#include "tinfra/test.h" // test infra

using namespace tinfra;

#if TINFRA_THREADS

SUITE(tinfra)
{
    using tinfra::thread::mutex;
    using tinfra::thread::condition;
    using tinfra::thread::thread;
    
    TEST(thread_mutex_basic)
    {
        mutex m;
        m.lock();
        m.unlock();
    }
    
    TEST(thread_condition_basic)
    {
        condition cond;
        cond.signal();
        cond.broadcast();
    }
    
    static int nothing_run_indicator = 0;
    static void* nothing(void*)
    {
        nothing_run_indicator = 1;
        return 0;
    }
        
    TEST(thread_simple)
    {
        thread t = thread::start(nothing, 0);
        CHECK_EQUAL(0, (intptr_t) t.join() );
        CHECK_EQUAL(1, nothing_run_indicator);
    }
    
    struct test_monitor {
	condition ca;
        condition cb;
	mutex m;
        bool started;
        bool finished;
    };
    
    static void* cond_signaler(void* p)
    {
        test_monitor* M = static_cast<test_monitor*>(p);
        
        // wait for start signal
        {
            guard g(M->m);
            while( !M->started )
                M->ca.wait(M->m); // wait for green light
        }
	for(int i = 0; i < 100*1000; i++ ) {
            std::ostringstream a;
            a << i*2;
            std::string x = a.str();
        }
        
        // signal finish!
        {
            guard g(M->m);
            M->finished = true;
            M->cb.signal(); // signal job finished
        }
        
        return 0;
    }
    static void* cond_waiter(void* p)
    {
        test_monitor* M = static_cast<test_monitor*>(p);
        {
            guard g(M->m);
            M->started = true;
            M->ca.signal(); // set up green light
        }
        
        {
            guard g(M->m);
            while( !M->finished )
                M->cb.wait(M->m); // wait for finish
        }
        return 0;
    }
    
    TEST(thread_condition)
    {
        test_monitor M;
        M.started = false;
        M.finished = false;
        thread p = thread::start(cond_signaler, &M);
        thread c = thread::start(cond_waiter, &M);
        CHECK_EQUAL(0, (intptr_t) c.join() );
        CHECK_EQUAL(0, (intptr_t) p.join() );
    }
    
    struct TestRunnable {
        int i;
        
        void operator()()
        {
            i = 1;
        }
    };
    
    TEST(thread_runnable)
    {
        TestRunnable runnable;
        runnable.i = 0;
        thread t = thread::start(tinfra::runnable_ref(runnable));
        t.join();
        CHECK_EQUAL(1, runnable.i);
    }
}

#endif // TINFRA_THREADS

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

