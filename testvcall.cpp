#include <iostream>

class F {
public:
    virtual void ifun(int, int) = 0;
};

class FImpl: public F {
    virtual void ifun(int a, int b)
    {
        std::cout << a+b << std::endl;
    }
};

void test(F* aaa)
{
	aaa->ifun(2,3);
}

int main()
{
	FImpl ggg;
	test(&ggg);
}
