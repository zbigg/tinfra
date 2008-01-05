#include <unittest++/UnitTest++.h>

#include "tinfra/exeinfo.h"
#include "tinfra/test.h"
#include <string>
#include <fstream>
#include <iostream>

using std::string;
using std::cout;
using std::endl;

    
SUITE(tinfra_exeinfo) {
    
    
    static void boo(tinfra::symbol_info const& syminfo) {
        //cout << (unsigned int)syminfo.address << " " << syminfo.name << endl;
    }
    
    TEST(symbol_map)
    {
        tinfra::test::TempTestLocation testLocation("unittests.exe.map");        
        string mappath = "unittests.exe.map";
        cout << "mappath: " << mappath << endl;
        std::ifstream input(mappath.c_str());
        CHECK( !!input );
        tinfra::read_symbol_map(input, boo);
    }
}
