#ifndef __tinfra_runner_h__
#define __tinfra_runner_h__

namespace tinfra {

class Runnable 
{
public:
    virtual ~Runnable() {}
    virtual void run() = 0;
};


class Runner 
{
public:
    virtual ~Runner() {}
    // TODO: invent&describe exception contract for runnable
    virtual void run(Runnable* runnable) = 0;
};

Runner&  getDefaultRunner();

} // end of namespace tinfra

#endif
