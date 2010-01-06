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

#include <algorithm>
#include <stdexcept>

#include <unittest++/UnitTest++.h>
#include <unittest++/TestReporter.h>
#include <unittest++/TestDetails.h>
#include <unittest++/TestResults.h>
#include <unittest++/TestReporter.h>

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

TINFRA_MODULE_TRACER(tinfra_test_fs_sandbox);

test_fs_sandbox::test_fs_sandbox(tstring const& name):
	fs_sandbox(tinfra::local_fs()),
	name_(name.str()) 
{
    TINFRA_USE_TRACER(tinfra_test_fs_sandbox);
    if( name_.size() > 0 ) {
        string real_path = path::join(top_srcdir, name_);
        if( !fs::exists(real_path) ) {
            throw std::logic_error(fmt("unable to find test resource %s (%s)") % name_ % real_path);
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

class TinfraTestReporter: public UnitTest::TestReporter {
public:    
    void ReportFailure(UnitTest::TestDetails const& details, char const* failure)
    {

#ifdef _MSC_VER 
        char const* const errorFormat = "%s(%d): error: FAILURE in %s: %s\n";        
#else   // only MSC has weird message format, rest use name:line: message (AFAIK)
        char const* const errorFormat = "%s:%d: error: FAILURE in %s: %s\n";
#endif
        out(errorFormat, details.filename, details.lineNumber, details.testName, failure);
    }

    void ReportTestStart(UnitTest::TestDetails const& test)
    {
        if( strcmp("DefaultSuite",test.suiteName) != 0 ) {
            out("TEST %s::%s\n", test.suiteName, test.testName);
        } else {
            out("TEST %s\n", test.testName);
        }
    }

    void ReportTestFinish(UnitTest::TestDetails const& /*test*/, float)
    {
    }

    void ReportSummary(int totalTestCount, int failedTestCount,
                       int failureCount, float secondsElapsed)
    {
        if (failureCount > 0)
            out("FAILURE: %d out of %d tests failed (%d failures).\n", failedTestCount, totalTestCount, failureCount);
        else
            out("Success: %d tests passed.\n", totalTestCount);
        out("Test time: %.2f seconds.\n", secondsElapsed);
    }
};

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
    bool operator()(const UnitTest::Test* const test) const
    {
        std::string test_name      = test->m_details.testName;
        std::string full_test_name = fmt("%s::%s") % test->m_details.suiteName % test_name;
        
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
    using UnitTest::TestList;
    using UnitTest::Test;
    const TestList& tl = Test::GetTestList();
    const Test* test = tl.GetHead();
    
    std::ostringstream tmp;
    tmp << "Available test cases:\n";
    
    while( test ) {
        std::string full_test_name = fmt("%s::%s") 
                                     % test->m_details.suiteName 
                                     % test->m_details.testName;
        tmp << "    " << full_test_name << "\n";
        test = test->next;
    }
    tinfra::out.write(tmp.str());
}

#ifdef SRCDIR
static std::string DEFAULT_TEST_RESOURCES_DIR = SRCDIR "/tests/resources";
#else
static std::string DEFAULT_TEST_RESOURCES_DIR =  "tests/resources";
#endif

tinfra::option<std::string> 
                      opt_test_resources_dir(DEFAULT_TEST_RESOURCES_DIR, 'D',"test-resources-dir", "set folder with file resources");
tinfra::option_switch opt_list('l', "test-list", "list available test cases");
tinfra::option_switch opt_help('h', "help", "show available options");

int test_main(int argc, char** argv)
{
    TinfraTestReporter reporter;
    UnitTest::TestRunner runner(reporter);
    
    std::vector<tinfra::tstring> args(argv+1, argv+argc);
    tinfra::option_registry::get().parse(args);
    
    if( opt_help.enabled() ) {
        using tinfra::path::basename;
        using tinfra::get_exepath;
        std::string usage_header = fmt(
            "Usage: %s [options] [ test_case ... ]\n"
            "Available options:\n") % basename(get_exepath());
        
        tinfra::out.write(tstring(usage_header));
        tinfra::option_registry::get().print_help(tinfra::out);
        return 0;
    }
    if( opt_list.enabled() ) {
        list_available_tests();
        return 0;
    }
    
    set_test_resources_dir(opt_test_resources_dir.value());

    test_name_list test_names(args.begin(), args.end());
        
    if( ! test_names.empty() ) {        
        test_name_matcher predicate(test_names);        
        return runner.RunTestsIf(UnitTest::Test::GetTestList(), 0, predicate, 0);
    } else {
        return runner.RunTestsIf(UnitTest::Test::GetTestList(), 0, UnitTest::True(), 0);
    }
}

} } // end namespace tinfra::test

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

