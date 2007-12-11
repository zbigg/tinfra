#include "tinfra/exception.h"
#include <iostream>

static void unhandled_exception(void)
{
    std::cout << "catched fatal exception" << std::endl;
}

int f(int a)
{
    if( a > 0 ) {
        return f(a-1);
    } else {
        char* p = (char*)a;
        *p = 0;
        return 0;
    }
}

void segv()
{
    f(4);
}

int main()
{
    tinfra::initialize_fatal_exception_handler(unhandled_exception);
    
    segv();
}
