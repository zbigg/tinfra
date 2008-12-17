//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <unittest++/UnitTest++.h>
#include <string>

#include "tinfra/fs.h"
#include "tinfra/path.h"
#include "tinfra/test.h"

using namespace tinfra;
    
SUITE(test_test) {
    
    TEST(test_TempTestLocation_empty)
    {
        std::string cwd = fs::pwd();
        {
            test::TempTestLocation testLocation;
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
            test::TempTestLocation testLocation("testtest_file");
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
            test::TempTestLocation testLocation("testtest_dir");
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
}
