#include <iostream>
#include <sys/time.h>

#include "timer.h"

namespace tinfra {

size_t timer::now()
{
    timeval tv;
    if( gettimeofday(&tv,0) == -1 ) 
        throw std::runtime_error("gettimeofday failed");
    
    size_t r = tv.tv_sec * 1000000 + tv.tv_usec;
    return r;
}

void timer::report()
{
    size_t end = _end;
    if( _running )
        end = now();
    int r = (end-_start);
    int s = r/1000000;
    int us = r % 1000000;
    std::cout << "time benchmark: " << _message << ": " <<  s << "." << std::setfill('0') << std::setw(6) << us << std::endl; 
}

} // end namespace tinfra
