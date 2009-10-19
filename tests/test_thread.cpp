//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/thread.h"
#include <sstream>

#include <unittest++/UnitTest++.h>

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
    
    struct A {
	condition c;
	mutex m;
	
	void signal() { 
	    m.lock();
	    c.signal();
	    m.unlock();
	}
	void wait()
	{
	    m.lock();
	    c.wait(m);
	    m.unlock();
	}
    };
    
    static void* cond_signaler(void* p)
    {
        A* a = static_cast<A*>(p);
	a->wait(); // wait for green light
	for(int i = 0; i < 100*1000; i++ ) {
            std::ostringstream a;
            a << i*2;
            std::string x = a.str();
        }
	a->signal(); // signal job finished
        return 0;
    }
    static void* cond_waiter(void* p)
    {
        A* a = static_cast<A*>(p);
	a->signal(); // set up green light
	a->wait(); // wait for finish
        return 0;
    }
    
    TEST(thread_condition)
    {
        A* a = new A;
	// TODO: this test is screwed up (seen on dual-core on win32)
	//      waiter if it's fast enough consume signal
	//      sent to signaler
	//      must use two synchronizers
        thread p = thread::start(cond_signaler, a);
        thread c = thread::start(cond_waiter, a);
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

