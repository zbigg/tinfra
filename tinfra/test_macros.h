        //
// Copyright (c) 2010, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_test_macros_h_included
#define tinfra_test_macros_h_included

#include "fmt.h"
#include "tstring.h" 
#include "trace.h" // for tinfra::source_location, TINFRA_SOURCE_LOCATION

namespace tinfra { 
namespace test {

/// Report test failure.
///
/// Reports a test failure described by message and source code location.
void report_test_failure(const char* filename, int line, const char* message);

#define SUITE(name) \
	namespace test_suite_##name { \
		inline const char* tinfra_test_suite_name() { return #name; } \
	} \
	namespace test_suite_##name

struct test_info {
	const char* name;
	const char* suite;
};

struct test_run_summary {
	int executed_test_count;
	int failed_test_count;
	int failure_count;
    float seconds_elapsed;
    
    test_run_summary(); // will 0 everything
};

class test_result_sink {
public:
	virtual ~test_result_sink();

	virtual void report_test_start(test_info  const&) = 0;
	virtual void report_failure(test_info const& info, tinfra::source_location const& location, tstring const& message) = 0;
	virtual void report_test_finish(test_info const&) = 0;
	virtual void report_summary(test_run_summary const&) = 0;
};

class default_test_result_sink: public test_result_sink {
public:
	default_test_result_sink();
	~default_test_result_sink();

	// test_result_sink interface
	void report_test_start(test_info  const&);
	void report_failure(test_info const& info, tinfra::source_location const& location, tstring const& message);
	void report_test_finish(test_info const&);
	void report_summary(test_run_summary const&);
};

class local_test_result_sink: public test_result_sink {
    test_result_sink* prev_sink;
    test_run_summary& result;
    bool              current_test_failed;
public:    
    local_test_result_sink(test_run_summary& r);
    ~local_test_result_sink();

    void report_test_start(test_info  const&);
    void report_failure(test_info const& info, tinfra::source_location const& location, tstring const& message);
    void report_test_finish(test_info const&);
    void report_summary(test_run_summary const&);

};

class test_base {
public:
	test_base(const char* suite_name, const char* test_name, tinfra::source_location const& source_location);
	~test_base();
	void run(test_result_sink& result);
protected:
	virtual void run_impl() = 0;
public:
	const char* name;
	const char* suite;
	tinfra::source_location source_location;
};

#define TEST(name) \
	class test_##name: public tinfra::test::test_base { \
	public: \
		test_##name(); \
	private: \
		virtual void run_impl(); \
	};\
	test_##name::test_##name(): \
	    tinfra::test::test_base(tinfra_test_suite_name(), #name, TINFRA_SOURCE_LOCATION()) \
    { } \
	test_##name test_instance_##name; \
	void test_##name::run_impl()

//
// equals
// equality_helper
//
template <typename T1, typename T2>
struct equality_helper {
	bool operator()(T1 const& a, T2 const& b) {
		return a == b;
	}
};

template <>
struct equality_helper<int, unsigned> {
	bool operator()(int a, unsigned b) {
		if( a < 0 ) {
			return false;
		} else {
			return unsigned(a) == b;
		}
	}
};

template <>
struct equality_helper<unsigned, int> {
	bool operator()(unsigned a, int b) {
		if( b < 0 ) {
			return false;
		} else {
			return unsigned(b) == a;
		}
	}
};

template <>
struct equality_helper<unsigned long, int> {
	bool operator()(unsigned long a, int b) {
		if( b < 0 ) {
			return false;
		} else {
			return (unsigned long)b == a;
		}
	}
};

template <>
struct equality_helper<int, unsigned long> {
	bool operator()(int a, unsigned long b) {
		if( a < 0 ) {
			return false;
		} else {
			return (unsigned long)a == b;
		}
	}
};


template <typename T1, typename T2>
bool equals(T1 const& a, T2 const& b)
{
	equality_helper<T1, T2> helper;
	return helper(a,b);
}

inline
bool equals(const char* a, const char* b)
{
	return tinfra::tstring(a) == tinfra::tstring(b);
}

//
// Check macros
//

/// Check basic equality
#define CHECK(predicate) \
	do {  if( !(predicate) ) { \
			std::string msg = tinfra::fmt("predicate %s failed") % #predicate; \
            ::tinfra::test::report_test_failure(__FILE__, __LINE__, msg.c_str()); \
		} \
	} while ( 0 );

/// Check basic equality
#define CHECK_EQUAL(expected, actual) \
        tinfra::test::check_equal(expected, actual, TINFRA_SOURCE_LOCATION())

/// Check value equality with tolerance
/// 
/// Checks if distance of between actual and expected is less or equal tollerancy.
/// 
/// TBD, the expressions are repeated, need to fix this in all
/// CHECK_... macros (see http://unittest-cpp.sourceforge.net/UnitTest++.html, Simple check macros)
// they have to be revamped as templates

#define CHECK_CLOSE(expected, actual, tollerancy) \
        tinfra::test::check_close(expected, actual, tollerancy, TINFRA_SOURCE_LOCATION())

#define CHECK_THROW(statement, exception_type) \
	do {  bool expected_exception_caught = false; \
		  try { statement; } \
		  catch( exception_type&) { \
			  expected_exception_caught = true; \
		  } \
		  catch( ... ) { \
			  std::string msg = "unexpected exception caught"; \
              ::tinfra::test::report_test_failure(__FILE__, __LINE__, msg.c_str()); \
			  throw; \
		  } \
		  if( !expected_exception_caught ) { \
			  std::string msg = tinfra::fmt("expected exception %s not caught") % #exception_type; \
              ::tinfra::test::report_test_failure(__FILE__, __LINE__, msg.c_str()); \
		  } \
	} while ( 0 )

//
// implementation details
//
template <typename T1, typename T2>
void check_equal(T1 const& expected, T2 const& actual, tinfra::source_location const& loc) 
{
    if( !tinfra::test::equals(expected,actual) ) {
	std::string msg = tinfra::fmt("expected '%s', but found '%s'") % (expected) % (actual);
        ::tinfra::test::report_test_failure(loc.filename, loc.line, msg.c_str());
    }
} 

template <typename T1, typename T2, typename T3>
void check_close(T1 const& expected, T2 const& actual, T3 const& tollerancy, tinfra::source_location const& loc) 
{
    const bool result = (expected >= actual)
                ? (expected - actual ) > tollerancy
                : (actual - expected ) > tollerancy;
    if( result ) {
	std::string msg = tinfra::fmt("expected %s (+/i %s) but found %s") % (expected) % (tollerancy) % (actual);
	::tinfra::test::report_test_failure(loc.filename, loc.line, msg.c_str());
    }
}

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
                           typename MapType::mapped_type const& value,
                           MapType const& map, std::string const& set_name, const char* filename, int line)
{
	typename MapType::const_iterator i = map.find(key);
	if( i == map.end() ) {
		std::string msg = tinfra::fmt("expected key (%s) not found in %s") 
			% key 
			% set_name;
		report_test_failure(filename, line, msg.c_str());
	}
	else if( !( i->second == value )) {
	    std::string msg = tinfra::fmt("entry '%s': expected '%s' but found '%s' ") 
			% key 
			% value
			% i->second;
	    report_test_failure(filename, line, msg.c_str());
	}
}



}}  // end namespace tinfra::test

inline const char* tinfra_test_suite_name() { return "default"; }

#endif // tinfra_test_macros_h_included
