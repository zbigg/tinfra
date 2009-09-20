//
// Copyright (C) 2008,2009 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

//
// mutex.h
//   tinfra::mutex definition
//

#ifndef tinfra_mutex_h_included
#define tinfra_mutex_h_included

#include <tinfra/platform.h>

#include <vector>

#if   defined( _WIN32)
// on win32 we must use pthread-win32 because we need condition variables
// #       include <tinfra/win32/thread.h>
#       define TINFRA_THREADS 1
#include <tinfra/win32/w32_mutex.h>
#elif defined(HAVE_PTHREAD_H)
#       include <tinfra/posix/posix_mutex.h>
#       define TINFRA_THREADS 1
#else
#error "tinfra: no threading support on this platform"
#       define TINFRA_THREADS 0
#endif

#endif // #ifdef tinfra_mutex_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

