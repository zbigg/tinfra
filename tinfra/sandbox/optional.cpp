//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <stdexcept>
#include <iostream>

class bad_optional: public std::runtime_error {
public:
    bad_optional(): std::runtime_error("trying to dereference empty optional value") {}
    bad_optional(std::string const& msg): std::runtime_error(msg) {}
};

template <typename T>
    class optional {
        T    _value;
        bool _exists;
    public:
        //struct nil {}
        optional(): _value(), _exists(false) {}
        
        optional(T const& value): _value(value), _exists(true) {}
        //optional(nil const&): _exists(false) {}
            
        bool exists() const { return _exists; }
        
        operator T const&() const { return get(); }
        operator T&() { return get(); }
        
        optional<T>& operator =(T const& value) {
            set(value);
            return *this;
        }
        
        T& get() {
            assertExists();
            return _value;
        }
        
        T const& get() const {
            assertExists();
            return _value;
        }
        
        void set(T const& value) {
            _value = value;
            _exists = true;
        }
        
    private:
        void assertExists() const {
            if( !exists() ) 
                throw bad_optional();
        }
    };
    
int main()
{
    try {
        const optional<int> a = 1;
        optional<int> c;
        optional<int> d(a);
        d = c;
        int z = d;
        c = 2;
        int x = a;
    } catch( std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
