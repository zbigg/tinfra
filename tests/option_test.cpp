//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/option.h" // API under test
#include "tinfra/test.h" // test infra

#include "tinfra/fail.h" // for fail
#include "tinfra/fmt.h" // for tsprintf
#include "tinfra/logger.h" // for log_handler_override
#include <string>

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
        
        CHECK( the_list.find_by_name("opt-1") == &opt1);
        CHECK( the_list.find_by_name("opt-2") == &opt2);
        CHECK( the_list.find_by_name("opt-3") == &opt3);
        
        
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
        
        params.push_back("--include");
        params.push_back("1");
        params.push_back("--include=2");
        params.push_back("-I3");
        params.push_back("-I");
        params.push_back("4");
        
        option_list the_list;
        
        list_option<int>  def(the_list, 'I', "include", "synopsis");
        
        CHECK_EQUAL( "synopsis", def.get_synopsis());
        CHECK( def.value().size() == 0 );
        
        the_list.parse(params);
        
        CHECK_EQUAL(0, params.size());
        
        CHECK(def.accepted());
        
        CHECK_EQUAL(4, def.value().size());
        
        CHECK_EQUAL(1, def.value()[0]);
        CHECK_EQUAL(2, def.value()[1]);
        CHECK_EQUAL(3, def.value()[2]);
        CHECK_EQUAL(4, def.value()[3]);
        
    }
    TEST(option_bug_string_with_space) {
        using std::vector;
        using std::string;
        
        using tinfra::tstring;
        using tinfra::option_list;
	using tinfra::list_option;
        using tinfra::option;

        vector<tstring> params;
        
        params.push_back("--def=abc def");
	params.push_back("--foo=zoo bar");
        
        option_list the_list;
        
        option<string>  def(the_list, "boom", "def", "foo_description");
	list_option<string> foo(the_list, "foo", "foo_description");

        the_list.parse(params);
        CHECK_EQUAL("abc def", def.value());
	CHECK_EQUAL(1, foo.value().size());
	CHECK_EQUAL("zoo bar", foo.value()[0]);
    }

    enum TestEnum {
        HIGH,
        LOW,
        UNSPECIFIED
    };
    
    std::ostream& operator<<(std::ostream& s, TestEnum v)
    {
        switch( v ) {
        case HIGH: return s << "high";
        case LOW: return s << "low";
        case UNSPECIFIED: return s << "unspecified";
        }
    }
    std::istream& operator>>(std::istream& s, TestEnum& v)
    {
        std::string tmp;
        s >> tmp;
             if( tmp == "high" ) v = HIGH;
        else if( tmp == "low" )  v = LOW;
        else if( tmp == "unspecified" ) v = UNSPECIFIED;
        else tinfra::fail("invalid input", 
                          tinfra::tsprintf("'%s' passed, but only high,low,unspecified is supported", tmp));
    }
    TEST(option_can_capture_enum_with_custom_ios_operators)
    {
         using tinfra::option_list;
         using tinfra::option;
         option_list the_list;
        
         option<TestEnum>  prio(the_list, UNSPECIFIED, 'p', "prio", "message priority");
         CHECK_EQUAL(false, prio.accepted());
         CHECK_EQUAL(UNSPECIFIED, prio.value());
         
         // check that it actually uses overloaded operator
         {
             std::vector<tinfra::tstring> params;
             params.push_back("--prio=high def");
             the_list.parse(params);
             
             CHECK_EQUAL(true, prio.accepted());
             CHECK_EQUAL(HIGH, prio.value());
        }
        
        // check that it "passws exception through"
        {
             tinfra::log_handler_override log_override;
             std::vector<tinfra::tstring> params;
             params.push_back("--prio=dupa def");
             CHECK_THROW( the_list.parse(params), std::runtime_error );
             
             //CHECK_EQUAL(true, prio.accepted());
             //CHECK_EQUAL(HIGH, prio.value());
         }
    }
} // end SUITE(tinfra)

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

