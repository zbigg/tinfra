#include <unittest++/UnitTest++.h>

#include "tinfra/exeinfo.h"

#include <string>
#include <fstream>
#include <iostream>

using std::string;
using std::cout;
using std::endl;

    
SUITE(tinfra_exeinfo) {
    
    
    static void boo(tinfra::symbol_info const& syminfo) {
        cout << (unsigned int)syminfo.address << " " << syminfo.name << endl;
    }
    
    TEST(symbol_map)
    {
        string exepath = tinfra::get_exepath();
        string mappath = exepath + ".map";
        cout << "mappath: " << mappath << endl;
        std::ifstream input(mappath.c_str());
        CHECK( !!input );
        tinfra::read_symbol_map(input, boo);
    }
}
