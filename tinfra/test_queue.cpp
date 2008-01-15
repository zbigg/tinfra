#include "tinfra/thread.h"
#include "tinfra/queue.h"
#include <sstream>

#include <unittest++/UnitTest++.h>

using namespace tinfra;

SUITE(test_queue)
{
    static void* produce(void* q_)
    {
        Queue<int>* q = (Queue<int>*)q_;
        for(int i = 0; i < 1000; ++i )
            q->put(i);
        return 0;
    }
    static void* consume(void* q_)
    {
        Queue<int>* q = (Queue<int>*)q_;
        for(int i = 0; i < 1000; ++i )
        {
            int r = q->get();
            if( r < 0 || r >= 1000 )
                throw UnitTest::AssertException("bad data in queue",__FILE__, __LINE__);
        }
        return 0;
    }
    static void* consume_check(void* q_)
    {
        Queue<int>* q = (Queue<int>*)q_;
        for(int i = 0; i < 1000; ++i )
        {
            if( i !=  q->get())
                throw UnitTest::AssertException("bad data in queue",__FILE__, __LINE__);
        }
        return 0;
    }
    
    TEST(queue_single_thread)
    {
        Queue<int> q;
        produce(&q);
        consume_check(&q);
        
    }
    
#if TINFRA_THREADS
    TEST(queue_2_threads)
    {
        Queue<int> q;
        ThreadSet ts;
        ts.start(&consume_check, &q);        
        
        ts.start(&produce, &q);
        
        ts.join();
    }
    
    TEST(queue_more_threads)
    {
        Queue<int> q;
        ThreadSet ts;
        ts.start(&consume, &q);        
        ts.start(&consume, &q);
        ts.start(&consume, &q);
        
        ts.start(&produce, &q);
        ts.start(&produce, &q);
        ts.start(&produce, &q);
        
        ts.join();
    }
#endif

}