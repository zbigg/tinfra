//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/option.h"

#include <string>

#include <unittest++/UnitTest++.h>

SUITE(tinfra) {
    TEST(option_api)
    {
        using tinfra::option_list;
        using tinfra::option_registry;
        using tinfra::option;
        
        option_registry::get();
        
        option_list the_list;
        
        option<int>         opt1(the_list, 99,      "opt-1", "opt_synopsis");
        option<std::string> opt2(the_list, "akuku", "opt-2", "opt_synopsis");
        
        CHECK( the_list.find_option("opt-1") == &opt1);
        CHECK( the_list.find_option("opt-2") == &opt2);
    }
} // end SUITE(fmt)
