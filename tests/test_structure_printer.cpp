//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/symbol.h"

#include <unittest++/UnitTest++.h>

#include "tinfra/mo.h"
#include "tinfra/structure_printer.h"

namespace tinfra_sp_test {
    TINFRA_SYMBOL_IMPL(x);
    TINFRA_SYMBOL_IMPL(y);
    struct point {
        int x;
        int y;
        TINFRA_MO_MANIFEST(point) {
            TINFRA_MO_FIELD(x);
            TINFRA_MO_FIELD(y);
        }
    };
}

namespace tinfra {
    template<> 
    struct mo_traits<tinfra_sp_test::point>: public tinfra::struct_mo_traits<tinfra_sp_test::point> {};
}

SUITE(tinfra)
{
    using tinfra_sp_test::point;
    
    TEST(structure_printer_struct) {
        std::ostringstream buf;
        tinfra::structure_printer printer(buf);
        point X = {1,2};
        tinfra::process(tinfra::symbol("X"), X, printer);
        CHECK_EQUAL("X={ x=1, y=2 }", buf.str());
    }
    
    TEST(structure_printer_container) {
        std::ostringstream buf;
        tinfra::structure_printer printer(buf);
        
        std::vector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        v.push_back(5);
        
        tinfra::process(tinfra::symbol("v"), v, printer);
        CHECK_EQUAL("v=[ 1, 2, 3, 5 ]", buf.str());
    }
    
    TEST(structure_printer_mo_container) {
        std::ostringstream buf;
        tinfra::structure_printer printer(buf);
        point X = {1,2};
        std::vector<point> v(2, X);
        tinfra::process(tinfra::symbol("v"), v, printer);
        CHECK_EQUAL("v=[ { x=1, y=2 }, { x=1, y=2 } ]", buf.str());
    }
    
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
