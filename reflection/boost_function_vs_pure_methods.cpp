#include <boost/function.hpp>

#include <iostream>

class iface {
public:
    virtual void foo(int b) = 0;
};

class impl: public iface {
    
    virtual void foo(int a) { std::cout << "ok\n";}
};

int main()
{
    boost::function<void (iface&, int)> x;
    x = &iface::foo;
    
    impl z;
    x(z, 2);
}
