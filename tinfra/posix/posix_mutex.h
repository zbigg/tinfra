//
// Copyright (C) 2008,2009 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_posix_mutex_h_included
#define tinfra_posix_mutex_h_included

#include <pthread.h>

namespace tinfra {

class mutex {
    pthread_mutex_t mutex_;
public:
    
    typedef pthread_mutex_t handle_type;

    mutex() { ::pthread_mutex_init(&mutex_, 0); }
    ~mutex() { ::pthread_mutex_destroy(&mutex_); }

    void lock() { 
        ::pthread_mutex_lock(&mutex_); 
    }
    void unlock() { 
        ::pthread_mutex_unlock(&mutex_); 
    }
    pthread_mutex_t* get_native() { return &mutex_; }
};

} // end namespace tinfra

#endif // #ifndef tinfra_posix_mutex_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
