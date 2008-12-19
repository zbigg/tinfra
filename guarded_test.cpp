#include "guarded_map.h"
#include <cassert>

#include <zthread/ThreadedExecutor.h>

const    int INVALID_VALUE = -1;
typedef  std::map<int,int> base_map_type;
typedef  guarded_map<int,int> test_map_type;

void assert_exists(test_map_type& v, int k)
{
    assert(v.contains(k));
    // check also view consistency
    
    int tmp;
    int r = v.get(k, INVALID_VALUE);
    
    assert( r != INVALID_VALUE);
    assert(v.find(k, tmp) == true);
    assert(tmp == r);
    assert(!v.empty());
}

void assert_missing(test_map_type& v, int k)
{
    assert(!v.contains(k));
    // check also view consistency
    
    int tmp;
    assert(v.get(k,INVALID_VALUE) == INVALID_VALUE);
    assert(v.find(k, tmp) == false);
    assert(v.remove(k) == false);
    assert(!v.contains(k));
}

void test_basic_opers(test_map_type& v)
{
    const int k = 100;
    if( !v.contains(k) ) {
        assert_missing(v, k);
        int s = v.size();
        v.put(k, k*2);
        assert(v.size() == s+1);
        assert(!v.empty());
    } else {
        assert(!v.empty());
    }
    assert_exists(v, k);
    
    assert(v.remove(k) == true);
    
    assert_missing(v, k);
}

#include <sstream>
#include <iostream>

#define LOGGA(a) do { std::ostringstream sss; sss << a << "\n"; std::cerr << sss.str(); } while(0)

int SHUTDOWN_ORDER = 9999;

class consumer: public ZThread::Runnable {
    test_map_type const& m_;
    int count_;
public:
    consumer(test_map_type& m, int count):
        m_(m), count_(count) {}
            
    virtual void run()
    {
        bool finished = false;
        while ( !finished) {
            //LOGGA("reader[" << count_ << "]  trying enter");
            ZThread::Thread::sleep(5);
            {
                test_map_type::read_guard g(m_);
                //LOGGA("reader[" << count_ << "]  processing ");
                ZThread::Thread::sleep(2);
            
                if( m_.contains(SHUTDOWN_ORDER) ) 
                    finished = true;
                LOGGA("reader[" << count_ << "]  finished");
            }
            
            
        }
        LOGGA("reader[" << count_ << "]  stopped");
    }
};

class producer: public ZThread::Runnable {
    test_map_type& m_;
    int count_;
public:
    producer(test_map_type& m, int count):
        m_(m), count_(count) {}
            
    virtual void run()
    {
        int z = 100;
        while ( z-- != 0 ) {
            
            LOGGA("writer: trying enter");
            {
                test_map_type::write_guard g(m_);
                LOGGA("writer: entered, working");
                
                ZThread::Thread::sleep(1);
                LOGGA("writer: exited");
            }
            ZThread::Thread::sleep(10);
        }
        LOGGA("writer: ordering shutdown");
        m_.put(SHUTDOWN_ORDER, 1);
    }
};


void test_rw_concurrency()
{
    ZThread::ThreadedExecutor spawner;
    
    test_map_type m;
    
    for( int i = 0; i < 200; ++i ) {
        spawner.execute( new consumer(m, i));
    }
    spawner.execute( new producer(m, 0));
    spawner.wait();
}

int main()
{
    test_map_type a;
    a.put(0,0);
    a.put(2,4);
    a.put(10,20);
    a.put(100,200);
    
    LOGGA("basic_opers_full" << 2);
    test_basic_opers(a);
    
    test_map_type empty;
    LOGGA("basic_opers_full");
    test_basic_opers(empty);
    
    LOGGA("rw_concurrency");
    test_rw_concurrency();
    return 0;
}
