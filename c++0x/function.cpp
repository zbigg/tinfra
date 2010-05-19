#include <functional>
#include <utility>

int g(int a)
{
    return a;
}

struct A {
    int f(int a) {
        return a;
    }
};

using namespace std::placeholders;

int main()
{
    using std::function;
    using std::bind;
    
    function<int (int)> a = g;
    A aa;
    
    a = bind(&A::f, aa, _1);
    
    return a(2);
}