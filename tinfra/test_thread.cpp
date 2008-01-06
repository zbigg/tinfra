#include "tinfra/thread.h"
#include <sstream>

#include <unittest++/UnitTest++.h>

using namespace tinfra;

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
        CHECK_EQUAL(0, (int) t.join() );
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
        Thread p = Thread::start(cond_signaler, a);
        Thread c = Thread::start(cond_waiter, a);
        CHECK_EQUAL(0, (int) c.join() );
        CHECK_EQUAL(0, (int) p.join() );
    }
    
}
