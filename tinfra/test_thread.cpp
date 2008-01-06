#include "tinfra/thread.h"
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
    
    void* cond_signaler(void* p)
    {
        Condition* cond = static_cast<Condition*>(p);
        cond->signal();
        return 0;
    }
    
    TEST(test_thread_cond)
    {
        Mutex m;
        Condition cond;
        Thread t = Thread::start(cond_signaler, &cond);
        m.lock();
        cond.wait(m);
        m.unlock();
        CHECK_EQUAL(0, (int) t.join() );
    }
}
