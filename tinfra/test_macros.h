//
// Copyright (c) 2010, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_test_macros_h_included
#define tinfra_test_macros_h_included

#include "fmt.h"

// we're currently basing on UnitTest++, so include them!
#include <unittest++/UnitTest++.h>
#include <unittest++/TestMacros.h>
#include <unittest++/CheckMacros.h>

namespace tinfra { 
namespace test {

/// Report test failure.
///
/// Reports a test failure described by message and source code location.
///
/// Currently UnitTest++ library current running test results is
/// updated with this failure.
void report_test_failure(const char* filename, int line, const char* message);

/// Check associative container equailty.
///
/// Please don't use directly, use CHECK_EQUAL_MAPS macro.
///
/// Check that two associative container are equal by means of content.
///
/// Actual map is checked item by item and each inconsistency:
///  - extra item
///  - value item under key not maching expected value
///  - missing items
/// is reported as separate failure message.
///
/// This function must be used from within TEST macro execution as it uses
/// static test infrastructure to report failures.
///
template <typename MapType>
void check_map_equal(MapType const& A, 
                     MapType const& B, std::string const& b_name, const char* filename, int line)
{
    typedef typename MapType::value_type value_type;
    typedef typename MapType::key_type   key_type;
    
    typedef typename MapType::const_iterator iter;
    
    for( iter ia = A.begin(); ia != A.end(); ++ia ) {
        iter ib = B.find(ia->first);
        if( ib == B.end() ) {
            std::string msg = tinfra::fmt("expected item (%s->%s) not found in %s") % ia->first % ia->second % b_name;
            report_test_failure(filename, line, msg.c_str()); 
        } else if ( !( ia->second == ib->second) ) {
            std::string msg = tinfra::fmt("expected (%s->%s) but found (%s->%s) found in %s") 
                % ia->first %ia->second 
                % ib->first %ib->second 
                % b_name;
            report_test_failure(filename, line, msg.c_str()); 
        }
    }
    
    for( iter ib = B.begin(); ib != B.end(); ++ib ) {
        iter ia = A.find(ib->first);
        if( ia == A.end() ) {
            std::string msg = tinfra::fmt("extra (%s->%s) found in %s") % ib->first %ib->second % b_name;
            report_test_failure(filename, line, msg.c_str()); 
        } 
    }
}

/// Check associative container equailty.
///
/// Check that two associative container are equal by means of content.
///
/// Actual map is checked item by item and each inconsistency:
///  - extra item
///  - value item under key not maching expected value
///  - missing items
/// is reported as separate failure message.
///
/// This macro must be used from within TEST macro execution as it uses
/// static test infrastructure to report failures.
///
/// @param expected    expected map contents (pattern)
/// @param actual      actual map contents to be checked against pattern

#define CHECK_EQUAL_MAPS(expected,actual) \
    tinfra::test::check_map_equal(expected, actual, #actual, __FILE__, __LINE__)

#define CHECK_SET_CONTAINS(expected_entry, container) \
    tinfra::test::check_container_contains(expected_entry, container, #container, __FILE__, __LINE__) 

#define CHECK_MAP_ENTRY_MATCH(expected_value, expected_key, map) \
    tinfra::test::check_map_entry_match(expected_key, expected_value, map, #map, __FILE__, __LINE__)

#define CHECK_STRING_CONTAINS(expected_substring, actual_result) \
    tinfra::test::check_string_contains(expected_substring, actual_result, #actual_result, __FILE__, __LINE__)

void check_string_contains(tstring const& expected_substring, tstring const& actual_result, tstring const& result_descr, const char* file, int line);

/*
#define CHECK_TYPES_EQUAL(expected_type, actual_type) \
    tinfra::test::check_string_contains(typeid(expected_type), typeid(actual_type), #actual_type, __FILE__, __LINE__)

void check_types_equal_contains(std::type_info const& expected_substring, std::type_info const& actual_result, tstring const& result_descr, const char* file, int line);
*/

// 
// implementation details
//

template <typename KeyType, typename SetType>
void check_container_contains(KeyType const& key, 
                     SetType const& set, std::string const& set_name, const char* filename, int line)
{
	typename SetType::const_iterator i = set.find(key);
	if( i == set.end() ) {
		std::string msg = tinfra::fmt("expected key (%s) not found in %s") 
			% key 
			% set_name;
		report_test_failure(filename, line, msg.c_str());
	}
}

template <typename MapType>
void check_map_entry_match(typename MapType::key_type const& key,
                           typename MapType::value_type const& value,
                           MapType const& map, std::string const& set_name, const char* filename, int line)
{
	typename MapType::const_iterator i = map.find(key);
	if( i == map.end() ) {
		std::string msg = tinfra::fmt("expected key (%s) not found in %s") 
			% key 
			% set_name;
		report_test_failure(filename, line, msg.c_str());
	}
	else if( !( *i == value )) {
	    std::string msg = tinfra::fmt("entry '%s': expected '%s' but found '%s' ") 
			% key 
			% value
			% *i;
	    report_test_failure(filename, line, msg.c_str());
	}
}



}}  // end namespace tinfra::test

#endif // tinfra_test_macros_h_included
