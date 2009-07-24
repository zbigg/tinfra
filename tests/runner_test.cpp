//
// Copyright (C) 2009 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/thread_pool.h"

#include "tinfra/trace.h"

#include <unittest++/UnitTest++.h>

SUITE(tinfra) {
    using tinfra::runner;
    using tinfra::runnable;
    using tinfra::runnable_ptr;
    
    void test(runner& r, runnable_ptr p, int N)
    {
	    for(int i = 0; i < N; ++i ) {
                r(p);
	    }
    };
    
    struct basic_recursive_job: public runnable {
        runner* parent_runner;
        int     n;
        static int runs;
        virtual void do_run()
        {
            runs++;
            if( n == 0 )
                return;
            basic_recursive_job next_job;
            next_job.parent_runner = parent_runner;
            next_job.n = n-1;
            (*parent_runner)( next_job );
        }
    };
    int basic_recursive_job::runs = 0;
    
    TEST(sequential_runner)
    {
        tinfra::sequential_runner runner;
        
        basic_recursive_job job;
        job.parent_runner = &runner;
        job.n = 5;
        basic_recursive_job::runs = 0;
        runner(job);
        CHECK_EQUAL(6, basic_recursive_job::runs);
    }
    /*
        tinfra::shared_ptr is not thread safe yet!!!
        so disable these tests
    TEST(thread_runner)
    {
        tinfra::thread_runner runner;
        
        basic_recursive_job job;
        job.parent_runner = &runner;
        job.n = 5; 
        runner(job);
    }
    
    TEST(static_thread_pool_runner)
    {
        tinfra::static_thread_pool_runner runner(5);
        
        basic_recursive_job job;
        job.parent_runner = &runner;
        job.n = 20; 
        runner(job);
    }*/
}

namespace tinfra {
    
runner::~runner() {}
runnable::~runnable() {}
sequential_runner::~sequential_runner() {}

};
// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

