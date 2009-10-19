//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
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

