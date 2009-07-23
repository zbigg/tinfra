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
    TEST(option_registry_api)
    {
        using tinfra::option_registry;
        option_registry::get();
    }
    
    TEST(option_api)
    {
        using std::string;
        using tinfra::option_list;
        using tinfra::option;
        using tinfra::option_switch;
            
        option_list the_list;
        
        option<int>    opt1(the_list, 99,      "opt-1", "opt_synopsis");
        option<string> opt2(the_list, "akuku", "opt-2", "opt_synopsis");
        option_switch  opt3(the_list,          "opt-3", "opt_synopsis");
        
        CHECK( the_list.find_option("opt-1") == &opt1);
        CHECK( the_list.find_option("opt-2") == &opt2);
        CHECK( the_list.find_option("opt-3") == &opt3);
        
        
        CHECK_EQUAL( opt1.default_value(), opt1.value());
        CHECK_EQUAL( opt2.default_value(), opt2.value());
        CHECK_EQUAL( opt3.default_value(), opt3.value());
        
        CHECK_EQUAL( 99,              opt1.value());
        CHECK_EQUAL( string("akuku"), opt2.value());
        CHECK_EQUAL( false          , opt3.value());
    }
    
    TEST(option_parse_simple) {
        using std::vector;
        using std::string;
        
        using tinfra::tstring;
        using tinfra::option_list;
        using tinfra::option;
        using tinfra::option_switch;

        vector<tstring> params;
        
        params.push_back("abc");
        params.push_back("--def=ijk");
        params.push_back("--foo");
        params.push_back("--bar=no");
        params.push_back("xyz");
        
        option_list the_list;
        
        option<string>  def(the_list, "boom", "def", "foo");
        option_switch   foo(the_list, "foo", "enable foo");
        option_switch   bar(the_list, "bar", "disable foo");
        
        the_list.parse(params);
        
        CHECK_EQUAL(2, params.size());
        
        CHECK_EQUAL("abc", params[0]);
        CHECK_EQUAL("xyz", params[1]);
        
        CHECK_EQUAL("ijk", def.value());
        CHECK_EQUAL(true,  foo.value());
        CHECK_EQUAL(false, bar.value());
    }
    
    TEST(option_bug_remove_last_param) {
        using std::vector;
        using std::string;
        
        using tinfra::tstring;
        using tinfra::option_list;
        using tinfra::option;
        using tinfra::option_switch;

        vector<tstring> params;
        
        params.push_back("--def=ijk");
        
        option_list the_list;
        
        option<string>  def(the_list, "boom", "def", "foo");
        the_list.parse(params);
        CHECK_EQUAL("ijk", def.value());
    }
    
    TEST(option_list_option)
    {
        using std::vector;
        using std::string;
        
        using tinfra::tstring;
        using tinfra::option_list;
        using tinfra::list_option;
        
        vector<tstring> params;
        
        params.push_back("--def=1");
        params.push_back("--def=2");
        params.push_back("--def=3");
        
        option_list the_list;
        
        list_option<int>  def(the_list, "def", "synopsis");
        
        CHECK_EQUAL( "synopsis", def.get_synopsis());
        CHECK( def.value().size() == 0 );
        
        the_list.parse(params);
        
        CHECK_EQUAL(0, params.size());
        
        CHECK(def.accepted());
        
        CHECK_EQUAL(3, def.value().size());
        
        CHECK_EQUAL(1, def.value()[0]);
        CHECK_EQUAL(2, def.value()[1]);
        CHECK_EQUAL(3, def.value()[2]);
        
    }
} // end SUITE(tinfra)

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

