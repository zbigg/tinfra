//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/exeinfo.h"
#include "tinfra/test.h" // for test infra

#include <string>
#include <fstream>
#include <iostream>

using std::string;
using std::cout;
using std::endl;

    
SUITE(tinfra) {
    
    
    static void boo(tinfra::symbol_info const&) {
        //cout << (unsigned int)syminfo.address << " " << syminfo.name << endl;
    }
    
    TEST(exeinfo_symbol_map)
    {
        tinfra::test::test_fs_sandbox testLocation("sample.exe.map");        
        string mappath = "sample.exe.map";
        std::ifstream input(mappath.c_str());
        CHECK( !!input );
        tinfra::read_symbol_map(input, boo);
    }
}
