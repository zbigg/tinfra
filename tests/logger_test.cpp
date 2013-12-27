//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/logger.h"  // we test this
#include "tinfra/test.h" // for test infra

SUITE(tinfra) {

    struct custom_log_handler: public tinfra::log_handler {
        int log_count;
        
        custom_log_handler(): log_count(0) {} 
        
        virtual void log(tinfra::log_record const&) { log_count++; }
    };
    
    TEST(log_handler_override)
        // check that log_handler_override
        // sucessfully catches all logs
    {
        using tinfra::log_handler;
        using tinfra::log_handler_override;
        
        log_handler* orig_lh = &(log_handler::get_default()); 
        CHECK( orig_lh != 0);
        
        custom_log_handler overriden;
        log_handler::set_default(&overriden);
        
        // check that manual override works
        {
            tinfra::log_error("fake");
            CHECK_EQUAL(1, overriden.log_count);
            tinfra::log_info("fake");
            CHECK_EQUAL(2, overriden.log_count);
        }
        {        
            log_handler_override lho;            
            CHECK( &log_handler::get_default() != &overriden);
            tinfra::log_error("fake");
            tinfra::log_info("fakse");
            CHECK_EQUAL(2, overriden.log_count); // still 2
        }
        
        {
            custom_log_handler overriden2;
            log_handler_override lho(overriden2);
            
            CHECK( &log_handler::get_default() == &overriden2 );
            tinfra::log_error("fake");
            tinfra::log_info("fakse");
            tinfra::log_info("fakse");
            CHECK_EQUAL(2, overriden.log_count); // still 2 in original handler
            CHECK_EQUAL(3, overriden2.log_count); // still 2 in original handler
        }
        
        log_handler::set_default(orig_lh);
    }
    
    struct empty_log_handler: public tinfra::log_handler {
        virtual void log(tinfra::log_record const&) {}
    };
    
    empty_log_handler elh;

    TEST(log_handler_api)
    {
        using tinfra::log_handler;
        log_handler* orig_lh = &(log_handler::get_default()); 
        CHECK( orig_lh != 0);
        
        // test set_default sets
        log_handler::set_default(&elh);
        CHECK_EQUAL(&elh, &(log_handler::get_default()));
        
        // .. and resets
        // note, this may now work if not default log handler
        // is used before test start :/
        log_handler::set_default(0);
        CHECK_EQUAL(orig_lh, &(log_handler::get_default()));
    }
    
    TEST(logger_api)
    {
        tinfra::logger log(elh);
        log.trace("foo");
        log.info("bar");
        log.warning("warning");
        log.error("error");
        log.fail("fail");
    }
    
    
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:



