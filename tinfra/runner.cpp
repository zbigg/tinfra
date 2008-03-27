#include "tinfra/runner.h"

namespace tinfra {
namespace {

class SynchronousRunner: public Runner {
public:
    virtual void run(Runnable* job)
    {
        job->run();
    }
};

} // end anon namespace

Runner&  getDefaultRunner()
{
    static SynchronousRunner defaultRunner;
    return defaultRunner;
}    

};
