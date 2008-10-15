#include <iostream>


#include "tinfra/thread.h"
#include "tinfra/runtime.h"
#include "tinfra/cmd.h"
#include "tinfra/fmt.h"

using namespace tinfra;

void* thread_fun(void* b)
{
    int a = (int)b;
    try {
        std::cerr << "thread " << a << " sleeping" << std::endl;        
        Thread::sleep(10 * 1000);
    } catch( interrupted_exception const& e) {
        std::cerr << "thread " << a << " sleep interrupted, exiting" << std::endl;
    }
    std::cerr << "thread " << a << " finishing" << std::endl;        
    return 0;
}

int posix_signals_main(int argc, char** argv)
{
    
    set_interrupt_policy(DEFERRED_SIGNAL);
    
    ThreadSet ts;
    
    for( int i = 0; i < 10 ; ++i ) {
        ts.start(&thread_fun, (void*)i);
    }
    
    ts.join();
    std::cerr << "master: done!" << std::endl;
    test_interrupt();
    std::cerr << "master: no interrupt" << std::endl;
    
    return 0;
}

TINFRA_MAIN(posix_signals_main);

