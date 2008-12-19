#include <iostream>
#include <stdexcept>

#include "tinfra/cmd.h"

#include "tstring.h"

using tinfra::tstring;

tstring badsanta()
{
    char buf[1024] = "anc";
    tstring a(buf);
    return a;
}
tstring dupaddd = "kkkk";

int test_param(tstring const& a)
{
    return a.size() > 0; 
}

void test_tstring()
{
    std::string ss = "hello";
    char buf[1024] = "hhh";
    
    tstring a = "zbyszek";
    tstring b = a;
    tstring c(b);
    std::string* ds = new std::string("yo");
    tstring d(*ds);
    tstring e(dupaddd);
    delete ds;
    
    test_param(ss);
    test_param("aa");
    test_param(buf);
}
void* test_in_other_thread(void* )
{
    test_tstring();
    return 0;
}
int stackstring_main(int argc, char** argv)
{
    test_tstring();
    //tinfra::ThreadSet ts;
    //ts.start(&test_in_other_thread,0);
    //ts.join();
    
    //ts.join();
    return 0;
}

TINFRA_MAIN(stackstring_main);

