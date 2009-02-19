//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_test_h__
#define __tinfra_test_h__

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
/// unittester. It invokes UnitTest++ unittests.
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
/// If used then UnitTest++ library must be linked with executable.
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

} } // end namespace tinfra::test

#endif
