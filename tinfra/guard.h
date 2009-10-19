//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
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

