//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_test_h_included
#define tinfra_test_h_included

#include "test_macros.h"

#include "tinfra/tstring.h"
#include "tinfra/fs_sandbox.h"
#include "tinfra/vfs.h"

namespace tinfra {
namespace test {

class test_fs_sandbox: public fs_sandbox {
public:
    explicit test_fs_sandbox(tstring const& name = "");
    explicit test_fs_sandbox(tinfra::vfs& vfs, tstring const& name = "");
    
    ~test_fs_sandbox();
    
private:
    std::string name_;
    std::string orig_pwd_;
};

void set_test_resources_dir(tstring const& d);

/// Test driver
///
/// Use this as your main function if building
/// unittester. It invokes tinfra-test 
/// 
/// If argument list is empty then it invokes all
/// unittests, if not. It invokes only tests
/// that are on argument list.
///
///
/// Test invocation is reported on stderr.
/// On win32 it's also reported to system debugger using OutputDebugString.
///
/// Example
///
///
/// Sample usage:
/// @code
///    int importer_main(int argc, char** argv)
///    {
///    #ifdef BUILD_UNITTEST
///        if( argc>1 && strcmp(argv[1], "--tests") )
///            return tinfra::test::test_main(argc,argv);
///    #endif
///        ...
///        normal processing
///    }
///  @endcode
///  @return 0 if all tests have passed, 1 if any of tests failed

int test_main(int argc, char** argv);

/// Expand $srcdir
///
/// Expand expression $srcdir to actual vaule passed 
/// via environment or --srcdir option.
std::string srcdir_eval(std::string const& path_expr);

} } // end namespace tinfra::test

#endif
