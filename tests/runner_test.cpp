//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/runner.h"
#include "tinfra/thread_runner.h"
#include "tinfra/test.h" // for test infra

#include "tinfra/trace.h"

SUITE(tinfra) {
    
    TINFRA_MODULE_TRACER(runner_test);
    
    using tinfra::runner;
    using tinfra::runnable_ptr;
    
    void test(runner& r, runnable_ptr p, int N)
    {
	    for(int i = 0; i < N; ++i ) {
                r(p);
	    }
    };
    
    tinfra::thread::monitor finish_monitor;
    
    struct basic_recursive_job  {
        runner* parent_runner;
        int     n;
        static int runs;
        static int finished;
        tinfra::shared_ptr<std::string> foo;
        void operator()()
        {
            {
                tinfra::thread::synchronizator sss(finish_monitor);
                
                if( n == 0 ) {
                    sss.signal();
                    //TINFRA_TRACE_MSG("finished!");
                    finished = 1;
                    return;
                }
                runs++;
            }
            basic_recursive_job next_job;
            next_job.parent_runner = parent_runner;
            next_job.n = n-1;
            next_job.foo = foo;
            (*parent_runner)( next_job );
        }
    };
    int basic_recursive_job::runs = 0;
    int basic_recursive_job::finished = 0;
    
    TEST(sequential_runner)
    {
        tinfra::sequential_runner runner;
        
        basic_recursive_job job;
        job.parent_runner = &runner;
        job.n = 1243;
        job.foo = tinfra::shared_ptr<std::string>(new std::string("jedziekonpobetonieitamsiatamadffhekfhkjdshfkjdhsfegwfyegfe"));
        basic_recursive_job::runs = 0;
        basic_recursive_job::finished = 0;
        runner(job);
        {
            tinfra::thread::synchronizator sss(finish_monitor);
            while( !basic_recursive_job::finished ) {
                sss.wait();
            }
        }
        CHECK_EQUAL(job.n, basic_recursive_job::runs);
    }
    
     //   tinfra::shared_ptr is not thread safe yet!!!
     //   so disable these tests
    TEST(thread_runner)
    {
        tinfra::thread_runner runner;
        
        basic_recursive_job job;
        job.parent_runner = &runner;
        job.n = 120; 
        {
            tinfra::thread::synchronizator sss(finish_monitor);
            basic_recursive_job::runs = 0;
            basic_recursive_job::finished = 0;
        }       
        runner(job);
        {
            tinfra::thread::synchronizator sss(finish_monitor);
            while( !basic_recursive_job::finished ) {
                sss.wait();
            }
        }
        CHECK_EQUAL(job.n, basic_recursive_job::runs);
    }
    
    TEST(static_thread_pool_runner)
    {
        tinfra::static_thread_pool_runner runner(5);
        
        basic_recursive_job job;
        job.parent_runner = &runner;
        job.n = 120; 
        {
            tinfra::thread::synchronizator sss(finish_monitor);
            basic_recursive_job::runs = 0;
            basic_recursive_job::finished = 0;
        }
        runner(job);
        
        {
            tinfra::thread::synchronizator sss(finish_monitor);
            while( !basic_recursive_job::finished ) {
                sss.wait();
            }
            CHECK_EQUAL(job.n, basic_recursive_job::runs);
        }
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:


