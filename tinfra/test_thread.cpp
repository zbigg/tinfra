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

SUITE(test_thread)
{
    TEST(test_mutex)
    {
        Mutex m;
        m.lock();
        m.unlock();
    }
    
    TEST(test_condition)
    {
        Condition cond;
        cond.signal();
        cond.broadcast();
    }
    void* nothing(void*)
    {
        return 0;
    }
    
    
    TEST(test_thread_simple)
    {
        Thread t = Thread::start(nothing, 0);
        CHECK_EQUAL(0, (intptr_t) t.join() );
    }
    struct A {
	Condition c;
	Mutex m;
	
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
    
    void* cond_signaler(void* p)
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
    void* cond_waiter(void* p)
    {
        A* a = static_cast<A*>(p);
	a->signal(); // set up green light
	a->wait(); // wait for finish
        return 0;
    }
    
    TEST(test_thread_cond)
    {
        A* a = new A;
	// TODO: this test is screwed up (seen on dual-core on win32)
	//      waiter if it's fast enough consume signal
	//      sent to signaler
	//      must use two synchronizers
        Thread p = Thread::start(cond_signaler, a);
        Thread c = Thread::start(cond_waiter, a);
        CHECK_EQUAL(0, (intptr_t) c.join() );
        CHECK_EQUAL(0, (intptr_t) p.join() );
    }
    
    struct TestRunnable: public Runnable {
        int i;
        
        void run()
        {
            i = 1;
        }
    };
    
    TEST(test_runnable)
    {
        TestRunnable runnable;
        runnable.i = 0;
        Thread t = Thread::start(runnable);
        t.join();
        CHECK_EQUAL(1, runnable.i);
    }
}
#endif // TINFRA_THREADS
