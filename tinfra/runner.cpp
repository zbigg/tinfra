//
// Copyright (C) 2009 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "runner.h"

namespace tinfra {
    
runner::~runner() {}
runnable_base::~runnable_base() {}

// WTF is with this language! ?
// why can't i just write
//   runnable runnable::EMPTY_RUNNABLE (0);
// or at least
//   runnable runnable::EMPTY_RUNNABLE (shared_ptr<runnable_ptr>(0));
static runnable_base* foo = 0;
static const shared_ptr<runnable_base> RUNNABLE_NULL(foo);
    
runnable runnable::EMPTY_RUNNABLE (RUNNABLE_NULL );
    
sequential_runner::~sequential_runner() {}

};

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

