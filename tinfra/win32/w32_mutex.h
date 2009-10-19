//
// Copyright (C) 2008,2009  Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

//
// win32/w32_mutex.h
//   win32 based implementation of mutex

#ifndef tinfra_win32_w32_mutex_h_included
#define tinfra_win32_w32_mutex_h_included

#define WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define _WIN32_WINNT 0x0500 // Windows 2000
#include <windows.h>

namespace tinfra {

class mutex {
public:    
    mutex();
    ~mutex();

    void lock();
    void unlock();

    CRITICAL_SECTION* get_native() { return &mutex_; };
private:
    CRITICAL_SECTION mutex_;
};

} // end namespace tinfra

#endif

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

