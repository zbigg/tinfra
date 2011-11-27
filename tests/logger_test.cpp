//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/logger.h"  // we test this
#include "tinfra/test.h" // for test infra

SUITE(tinfra) {
    
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



