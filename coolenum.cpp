#include <cassert>
template <typename ES>
struct CoolEnum {
    static bool ok(int a) {
        return a >= ES::FIRST && a < ES::LAST;
    }

    CoolEnum(): 
        value(ES::FIRST)
    {}
     
    //operator typename ES::value_type() const { 
    //    return this->value; 
    //}

    CoolEnum& operator=(typename ES::value_type const& x)  {
        this->value = x;
    }
private:
    typename ES::value_type value;
};

template <typename T>
bool operator ==(typename T::value_type a, CoolEnum<T> b) {
    return a == T::value_type(b);
}

template <typename T>
bool operator ==(CoolEnum<T> a, typename T::value_type b) {
    return b == T::value_type(a);
}



struct A: public CoolEnum<A> {
enum value_type {
    FIRST = 0,
    FOO   = 0,
    BAR   = 1,
    END   = 2
};
};

int main()
{
    assert( !A::ok(-1) );
    assert(  A::ok(A::FOO) );
    assert(  A::ok(A::BAR) );
    assert( !A::ok(2) );
    assert( !A::ok(3) );
    
    A foo;
    assert( A == A::FIRST);
}
