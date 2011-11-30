//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/platform.h"

#include "tinfra/test.h"

#include "tinfra/fs.h"
#include "tinfra/path.h"
#include "tinfra/fmt.h"
#include "tinfra/trace.h"
#include "tinfra/option.h"
#include "tinfra/stream.h"
#include "tinfra/exeinfo.h"
#include "tinfra/runtime.h" // for debug_info
#include "cli.h"
#include <algorithm>
#include <stdexcept>
#include <stdio.h> // screw cxxx headers because they are incompatible
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#endif

namespace tinfra {
namespace test {

using std::string;
    
#ifdef SRCDIR
    #define DEFAULT_TOP_SRCDIR SRCDIR
#else
    #define DEFAULT_TOP_SRCDIR "."
#endif

static std::string top_srcdir = DEFAULT_TOP_SRCDIR;

//
// test_fs_sandbox
//
TINFRA_MODULE_TRACER(tinfra_test_fs_sandbox);

test_fs_sandbox::test_fs_sandbox(tstring const& name):
	fs_sandbox(tinfra::local_fs()),
	name_(name.str()) 
{
    TINFRA_USE_TRACER(tinfra_test_fs_sandbox);
    if( name_.size() > 0 ) {
        string real_path = path::join(top_srcdir, name_);
        if( !fs::exists(real_path) ) {
			const std::string error_message = (fmt("unable to find test resource %s (%s)") % name_ % real_path).str();
            throw std::logic_error(error_message);
        }
        
        fs::recursive_copy(real_path, fs_sandbox::path());
    } 
    orig_pwd_ = fs::pwd();
    fs::cd(fs_sandbox::path());
    TINFRA_TRACE_MSG(fmt("entering sandbox pwd='%s'") % fs_sandbox::path());
}
test_fs_sandbox::~test_fs_sandbox()
{
    TINFRA_USE_TRACER(tinfra_test_fs_sandbox);
    TINFRA_TRACE_MSG(fmt("leaving sandbox pwd='%s'") % orig_pwd_);
    fs::cd(orig_pwd_);    
}

void set_test_resources_dir(tstring const& x)
{
    top_srcdir.assign(x.data(), x.size());
}

//
// test_result_sink
//
test_result_sink::~test_result_sink()
{
}

//
// test_base
//

test_base::test_base(const char* _suite_name, const char* test_name):
	suite(_suite_name),
	name(test_name)
{
	static_registry<test_base>::register_element(this);
}

test_base::~test_base()
{
	static_registry<test_base>::unregister_element(this);
}

// globals to access "current test result reporter"
test_base*              currently_executed_test = 0;
test_result_sink*       current_test_result_sink = 0;

tinfra::trace::location last_seen_source_location;

void test_base::run(test_result_sink& sink)
{
	test_info info;
	info.name = this->name;
	info.suite = this->suite;
	sink.report_test_start(info);
	currently_executed_test = this;
	current_test_result_sink = &sink;
	try {
		this->run_impl();
	} catch( std::exception& e ) {
		std::string const message = (fmt("exception thrown: %s") % e.what()).str();
		sink.report_failure(info, last_seen_source_location, message);
		currently_executed_test = 0;
		current_test_result_sink = 0;
	} catch( ... ) {
		std::string const message = "unknown exception thrown";
		sink.report_failure(info, last_seen_source_location, message);

		currently_executed_test = 0;
		current_test_result_sink = 0;
		sink.report_test_finish(info);
		throw;
	}
	sink.report_test_finish(info);
}


static void out(const char* message, ...)
{
    va_list ap;
    va_start(ap, message);
    char buf[2048];
    vsprintf(buf,message, ap);
    //printf("%s",buf);
    tinfra::out.write(buf);
#ifdef _WIN32
    OutputDebugString(buf);
#endif
    va_end(ap);
}

// default_test_result_sink
default_test_result_sink::default_test_result_sink()
{
}
default_test_result_sink::~default_test_result_sink()
{
}

	// test_result_sink interface
void default_test_result_sink::report_test_start(test_info  const& info)
{
	if( strcmp("DefaultSuite",info.suite) != 0 ) {
        out("TEST %s::%s\n", info.suite, info.name);
    } else {
        out("TEST %s\n", info.name);
    }
}
void default_test_result_sink::report_failure(test_info const& info, tinfra::trace::location const& location, tstring const& message)
{
#ifdef _MSC_VER 
    char const* const errorFormat = "%s(%d): error: FAILURE in %s: %s\n";        
#else   // only MSC has weird message format, rest use name:line: message (AFAIK)
    char const* const errorFormat = "%s:%d: error: FAILURE in %s: %s\n";
#endif
	const char* name = info.name;
	tinfra::string_pool pool;
    out(errorFormat, location.filename, location.line, name, message.c_str(pool));
}

void default_test_result_sink::report_test_finish(test_info const&)
{
}

void default_test_result_sink::report_summary(test_run_summary const& summary)
{
	if ( summary.failure_count > 0) {
		out("FAILURE: %d out of %d tests failed (%d failures).\n", 
			summary.failed_test_count, 
			summary.executed_test_count, 
			summary.failure_count);
	} else {
		out("Success: %d tests passed.\n", summary.executed_test_count);
	}
	// TODO, no support for performance counter yet :/
	//out("Test time: %.2f seconds.\n", summary.seconds_elapsed);
}

//
// globally accessible test failure report API
//
void report_test_failure(const char* filename, int line, const char* message)
{
	if( !current_test_result_sink )
		return;
	
	tinfra::trace::location location;
	location.filename = filename;
	location.line = line;
	location.name = "";

	test_info info = { "", "" };
	if( currently_executed_test ) {
		info.name = currently_executed_test->name;
		info.suite = currently_executed_test->suite;
	}

	current_test_result_sink->report_failure(info, location, message);
}

//
// test runner
// 

typedef std::vector<std::string> test_name_list;

class test_name_matcher {
    test_name_list& names_;
public:
    test_name_matcher(test_name_list& n): names_(n) {}
    
    static bool matches(tstring const& mask, tstring const& str)
    {
        if( mask == str )
            return true;
        if( mask.size() > 0 && mask[mask.size()-1] == '*') {
            size_t const_part_len = mask.size()-1;
            
            if( str.size() >= const_part_len &&
                str.substr(0,const_part_len) == mask.substr(0, const_part_len) )
                return true;
        }
        return false;
    }
    bool operator()(test_base const& test) const
    {
        std::string test_name      = test.name;
        std::string full_test_name = fmt("%s::%s") % test.suite % test_name;
        
        for( test_name_list::const_iterator i = names_.begin(); i != names_.end(); ++i )
        {
            std::string const& mask = *i;
            if( matches(mask, test_name) ) 
                return true;
            if( matches(mask, full_test_name) ) 
                return true;
        }
        return false;
    }
};

static void list_available_tests()
{
	std::vector<test_base*> const& tests  = static_registry<test_base>::elements();

	std::ostringstream out;
    out << "Available test cases:\n";
	for( std::vector<test_base*>::const_iterator i = tests.begin(); i != tests.end() ; ++i ) {
		test_base const& test = **i;
		std::string full_test_name = fmt("%s::%s") 
                                       % test.name 
                                       % test.suite;
        out << "    " << full_test_name << "\n";
	};

    tinfra::out.write(out.str());
}

class intermediate_test_result_sink: public test_result_sink {
	test_result_sink& next;
public:
	int failures;
	float seconds_elapsed;
	float start_time;

	intermediate_test_result_sink(test_result_sink& n):
	    next(n),
		failures(0),
		seconds_elapsed(0.0),
		start_time(0.0)
	{
	}
	~intermediate_test_result_sink()
	{
	}

	// test_result_sink interface
	void report_test_start(test_info  const& ti)
	{
		this->next.report_test_start(ti);
		start_time = now();
	}
	void report_failure(test_info const& info, tinfra::trace::location const& location, tstring const& message)
	{
		this->next.report_failure(info, location, message);
		this->failures++;
	}
	void report_test_finish(test_info const& ti)
	{

		seconds_elapsed = now() - this->start_time;
		this->next.report_test_finish(ti);
	}
	void report_summary(test_run_summary const& ti)
	{
		this->next.report_summary(ti);
	}
private:
	float now() const {
		// TODO, no support for performance counter yet :/
		return 0.0;
	}
};

#ifdef SRCDIR
static std::string DEFAULT_TEST_RESOURCES_DIR = SRCDIR "/tests/resources";
#else
static std::string DEFAULT_TEST_RESOURCES_DIR =  "tests/resources";
#endif

tinfra::option<std::string> 
                      opt_test_resources_dir(DEFAULT_TEST_RESOURCES_DIR, 'D',"test-resources-dir", "set folder with file resources");
tinfra::option_switch opt_list('l', "test-list", "list available test cases");

int test_main_real(tstring const&, std::vector<tinfra::tstring>& args)
{
    if( opt_list.enabled() ) {
        list_available_tests();
        return 0;
    }
    
    set_test_resources_dir(opt_test_resources_dir.value());

    test_name_list test_names(args.begin(), args.end());
    test_name_matcher predicate(test_names);

    std::vector<test_base*> const& tests  = static_registry<test_base>::elements();

	default_test_result_sink result_sink;
	test_run_summary summary;
	summary.executed_test_count = 0;
	summary.failed_test_count = 0;
	summary.failure_count = 0;
	summary.seconds_elapsed = 0;

	for( std::vector<test_base*>::const_iterator i = tests.begin(); i != tests.end() ; ++i ) {
		test_base& test = **i;
		if( test_names.empty() || predicate(test) ) {
			intermediate_test_result_sink int_sink(result_sink);
			test.run(int_sink);
			if( int_sink.failures > 0 ) {
				summary.failed_test_count++;
				summary.failure_count += int_sink.failures;
			}
			summary.executed_test_count++;
			summary.seconds_elapsed += int_sink.seconds_elapsed;
		}
	}
	result_sink.report_summary(summary);

	const int exit_code = summary.failed_test_count > 0 ? 1 : 0;
	return exit_code;
}

int test_main(int argc, char** argv)
{
    return tinfra::cli_main(argc, argv, &test_main_real);
}

void check_string_contains(tstring const& expected_substring, 
    tstring const& actual_result, 
    tstring const& result_descr, 
    const char* filename, int line)
{
    if( actual_result.find(expected_substring) != tstring::npos )
        return;
    
    const std::string msg = tinfra::fmt("expected substring '%s' not found in actual value '%s': '%s'") 
        % expected_substring
        % result_descr
        % actual_result;
    report_test_failure(filename, line, msg.c_str()); 
}

} } // end namespace tinfra::test

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

