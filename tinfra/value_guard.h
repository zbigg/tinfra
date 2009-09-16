//
// Copyright (C) 2009 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_value_guard_h_included
#define tinfra_value_guard_h_included

namespace tinfra {

template <typename T>
class value_guard {
    T& ref;
    T  copy;
public:
    value_guard(T& victim):
        ref(victim),
        copy(victim)
    {
    }
    
    ~value_guard() {
        if( !( ref == copy ) ) {
            ref = copy;
        }
    }
};

} // end namespace tinfra

#endif // tinfra_value_guard_h_included

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:

