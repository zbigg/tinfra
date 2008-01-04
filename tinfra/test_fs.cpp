#include "tinfra/fs.h"
#include "tinfra/path.h"
#include <iostream>

#include <unittest++/UnitTest++.h>

using namespace tinfra::fs;
using namespace tinfra::path;

SUITE(tinfra_fs)
{
    TEST(test_copy)
    {
        copy("libtinfra.a", "libtinfra.b");
    }
    
    TEST(test_list_files)
    {
        std::vector<std::string> files;
        list_files("tinfra", files);
        CHECK( files.size() > 0 );
        for( std::vector<std::string>::const_iterator i = files.begin(); i!=files.end(); ++i )
        {
            std::cout << *i << std::endl;
        }
    }
    
    TEST(test_mkdir_rmdir)
    {
        {
            const char* name = "kukkuryku";
            CHECK( !exists(name));
            mkdir(name);
            CHECK( exists(name));
            CHECK( is_dir(name));
            rmdir(name);
            CHECK( !is_dir(name));
        }
        
        {
            const char* name = "huzia/c/f/e";
            CHECK( !exists(name));
            mkdir(name,true);
            CHECK( exists(name));
            CHECK( is_dir(name));
            rmdir("huzia/c/f/e");
            rmdir("huzia/c/f");
            rmdir("huzia/c");
            rmdir("huzia");
            CHECK( !exists("a"));
        }
    }
    TEST(test_recursive)
    {
        recursive_copy("tinfra", "boo");
        recursive_rm("boo");
    }
}
