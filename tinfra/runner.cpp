//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

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
