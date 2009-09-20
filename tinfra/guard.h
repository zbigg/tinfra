//
// Copyright (C) 2008,2009 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_guard_h_included
#define tinfra_guard_h_included

#include "mutex.h"

namespace tinfra {

class guard {
    mutex& m;
public:
    guard(mutex& pm): m(pm)
    {
        m.lock();
    }
    ~guard()
    {
	m.unlock();
    }
};

} // end namespace tinfra

#endif // #ifdef tinfra_guard_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

