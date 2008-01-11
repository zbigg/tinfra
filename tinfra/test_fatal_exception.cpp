#include "tinfra/exception.h"
#include "tinfra/cmd.h"
#include <iostream>

#define FATAL_EXCEPTION_MESSAGE "test_fatal_exception: SUCCESS catched fatal exception"
static void fatal_exception_handler(void)
{
    std::cout << FATAL_EXCEPTION_MESSAGE << std::endl;
}

int f(long a)
{
    if( a > 0 ) {
        return f(a-1);
    } else {
        char* p = 0;
        *p = 0;
        return 0;
    }
}

void segv()
{
    f(4L);
}

void test_segv(tinfra::cmd::app& app)
{
    app.inform("this script should fail with stack trace\n"
               "and message \"" FATAL_EXCEPTION_MESSAGE "\"");
    segv();
}

#ifdef _WIN32
#include <windows.h>
#define sleep(a) Sleep(a*1000)
#else
#include <unistd.h>
#endif

// TODO: differentiate between interrupt (keyboard/Ctrl+C)
// and TERM
// TERM is sent to window on x-window, it's default signal to kill ?
// What is sent to window on windows ?
// INT is sent on Ctrl+C (unix/windows)
//
void test_terminate()
{
    
}

struct simple_raii {
    tinfra::cmd::app& app;
    simple_raii(tinfra::cmd::app& a): app(a) {}
    ~simple_raii() {
        app.inform("SUCCESS destructor called");
    }
};

void test_interrupt(tinfra::cmd::app& app)
{
    app.inform("you have 10 seconds to interrupt program");
    simple_raii destructor_tester(app);
    sleep(10);
}
int test_main(int argc, char** argv)
{    
    tinfra::cmd::app& app = tinfra::cmd::app::get();
    tinfra::set_fatal_exception_handler(fatal_exception_handler);
    
    if( argc < 2 ) app.fail("test name required\nsegv,int\n");
    std::string test_name = argv[1];
    if( test_name == "segv" )
        test_segv(app);
    //else if( test_name == "term" )
    //    test_terminate();
    else if( test_name == "int" )
        test_interrupt(app);
    else 
        app.fail("unknown test type");
    return 0;
}

int main(int argc,char** argv)
{
    return tinfra::cmd::main(argc,argv,test_main);
}
