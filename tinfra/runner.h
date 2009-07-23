//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_runner_h_included
#define tinfra_runner_h_included

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
