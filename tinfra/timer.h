#ifndef __tinfra_timer_h__
#define __tinfra_timer_h__

#include <string>

namespace tinfra {
    
class timer {
    std::string _message;
    size_t      _start;
    size_t      _end;
    bool        _running;
public:
    timer(std::string const& message): 
        _message(message),
        _start(0),
        _end(0),
        _running(false)
    {
        start();
    }
    ~timer()
    {
        if( !stopped()) {
            stop();
            report();
        }
    }
    
    void start()
    {
        _start = now();
        _running = true;
    }
    void stop()
    {
        _end = now();
        _running = false;
    }
    

    bool stopped() const { return ! _running; }
    void report();
    
private:
    size_t now();
};

}

#endif // __tinfra_timer_h__