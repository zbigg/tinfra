#include <pcre.h>

#include <string>

#include <iostream>
#include <assert.h>

#include "tinfra/string.h"
#include "tinfra/cmd.h"

#include "regex.h"

//
// sample program, proof of concept
//

using tinfra::regexp;
using tinfra::scanner;
using tinfra::matcher;
void test_scanner()
{
    std::string name;
    int h,m,s;
    bool matches = scanner("^(\\w+) (\\d+):(\\d+):(\\d+)$", "Week 1:22:333") % name % h % m % s;
    
    assert(matches);
    assert(name=="Week");
    assert(h==1);
    assert(m==22);
    assert(s==333);
}
int regexp_pcre_main(int argc, char** argv)
{
    test_scanner();
    
    regexp re(argv[1]);
    std::string line;
    
    while( std::getline(std::cin, line) ) {
        tinfra::strip_inplace(line);
        for(matcher m(re, line.c_str(), line.size()); m.has_next(); ) {
            regexp::match_result const& match = m.next();
            std::cout << match[0] << std::endl;
        }
    }
    return 0;    
}

TINFRA_MAIN(regexp_pcre_main);
