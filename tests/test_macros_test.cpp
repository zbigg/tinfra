//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/test.h" // for test infra

#include "tinfra/trace.h"

SUITE(tinfra_test) {
    
     #define CHECK_THAT_IT_FAILS(times, statement) \
       { tinfra::test::test_run_summary tr; \
           { tinfra::test::local_test_result_sink failure_catcher(tr); \
             statement; \
           } \
         CHECK_EQUAL(times, tr.failure_count); \
       }
       
    TEST(check_equal_basics)
    {
        CHECK_EQUAL(0,0);
        CHECK_EQUAL("1","1");
        CHECK_EQUAL("","");
        
        CHECK_THAT_IT_FAILS(1, CHECK_EQUAL(0,1)); 
    }
    
    TEST(check_equal_evaluates_once)
    {
        // in case of success
        {
            int i = 0;
            CHECK_EQUAL(0, i++);
            CHECK_EQUAL(1, i);
        }
        
        // in case of failure
        {
            int i = 0;
        
            CHECK_THAT_IT_FAILS(1, CHECK_EQUAL(1, i++));
            CHECK_EQUAL(1, i);
        }
    }
    
   
       
    TEST(check_close_basics)
    {
        CHECK_CLOSE(0, 0, 0);
        CHECK_CLOSE(0, 1, 1);
        CHECK_CLOSE(0, -1, 1);
        
        CHECK_THAT_IT_FAILS(1, CHECK_CLOSE(0,1,0));
        CHECK_THAT_IT_FAILS(1, CHECK_CLOSE(1,0,0));
    }
    
    TEST(check_close_evaluates_once)
    {
        // in case of success
        {
            int i = 0;
            CHECK_CLOSE(0, i++, 0);
            CHECK_EQUAL(1, i);
        }
        
        // in case of failure
        {
            int i = 0;
            CHECK_THAT_IT_FAILS(1, CHECK_CLOSE(1, i++, 0));
            
            CHECK_EQUAL(1, i);
        }
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:


