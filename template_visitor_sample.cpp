#include <iostream>

#define TRACE(x) std::cout << __FILE__ << ":" << __LINE__ << ": " << #x << "=" << x << "\n";

struct P {
    int x;
    int y;
    
    template <typename F>
    void process()(const char* name, F& f) const
    {
        f("x", x);
        f("y", y);
    }
    
    
};

struct R {
    P lr;
    P tb;
    
    template <typename F>
    void process()(const char* name, F& f) const
    {
        f("lr", lr);
        f("tb", tb);
    }
};

//
//
//


template <typename T>
struct traits {
    template <template F>
    void process(const char* name, T& value, F& f)
    {
        f(name, value);
    }
};

template <typename T>
struct struct_traits {
    template <template F>
    void process(const char* name, T& value, F& f)
    {
        value.process(f);
    }
};

template <>
struct traits<P>: public struct_traits<P>
//
//
//

int main()
{
    int x = 22;
    TRACE(x);
}
