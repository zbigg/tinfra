#include <cassert>

template <typename T>
class reference_wrapper {
public:
    reference_wrapper(T& p):
        p_(&p)
    {
    }
    
    operator T&() { return *p_; }
private:
    T* p_;
};

struct foo {
    int a;
    void operator()() {
        a = 1;
    }
};

template <typename T>
void bar(T fun)
{
    fun();
}


int main()
{
    foo a = { 0 };
    reference_wrapper<foo> rw(a);

    bar(rw);
    assert(a.a == 1);
    return 0;
}