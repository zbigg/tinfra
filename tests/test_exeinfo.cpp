//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <unittest++/UnitTest++.h>

#include "tinfra/exeinfo.h"
#include "tinfra/test.h"
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
        tinfra::test::TempTestLocation testLocation("sample.exe.map");        
        string mappath = "sample.exe.map";
        std::ifstream input(mappath.c_str());
        CHECK( !!input );
        tinfra::read_symbol_map(input, boo);
    }
}
