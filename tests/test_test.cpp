//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include <string>

#include "tinfra/fs.h"
#include "tinfra/path.h"
#include "tinfra/test.h"

#include "tinfra/test.h" // test infra

using namespace tinfra;
    
SUITE(tinfra) {
    
    TEST(test_TempTestLocation_empty)
    {
        std::string cwd = fs::pwd();
        {
            tinfra::test::test_fs_sandbox testLocation;
            CHECK(cwd != fs::pwd());
            std::vector<std::string> files = fs::list_files(".");
            CHECK_EQUAL(0, int(files.size()));
        }
        CHECK_EQUAL(cwd, fs::pwd());
    }
    
    TEST(test_TempTestLocation_file)
    {
        std::string cwd = fs::pwd();
        {
            tinfra::test::test_fs_sandbox testLocation("testtest_file");
            CHECK(cwd != fs::pwd());
            std::vector<std::string> files = fs::list_files(".");
            CHECK_EQUAL(1, int(files.size()));
            CHECK_EQUAL("testtest_file", files[0]);
        }
        CHECK_EQUAL(cwd, fs::pwd());
    }
    
    TEST(test_TempTestLocation_dir)
    {
        std::string cwd = fs::pwd();
        {
            tinfra::test::test_fs_sandbox testLocation("testtest_dir");
            CHECK(cwd != fs::pwd());
            std::vector<std::string> files = fs::list_files(".");
            CHECK_EQUAL(1, int(files.size()));
            CHECK_EQUAL("testtest_dir", files[0]);
            CHECK( fs::is_dir("testtest_dir"));
            CHECK( fs::is_dir("testtest_dir/a"));        
            CHECK( fs::is_file("testtest_dir/file1"));
            CHECK( fs::is_file("testtest_dir/a/file2"));
        }
        CHECK_EQUAL(cwd, fs::pwd());
    }
    
    TEST(test_srcdir_eval)
    {
        std::string result = tinfra::test::srcdir_eval("$srcdir/tests/test_test.cpp");
        CHECK( result.find("$srcdir") == std::string::npos);
        
        CHECK( tinfra::fs::is_file(result));
    }
}
