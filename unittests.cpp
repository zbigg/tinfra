#include <unittest++/UnitTest++.h>
#include <iostream>
using namespace std;

struct foo {
    int a;
    foo(int a): a(a) {
	cerr << "foo " << a << " created" << endl;
    }
    ~foo() {
	cerr << "foo " << a << " destroyed" << endl;
    }
};

void handler()
{
    cerr << "handler" << endl;
    return;
}

bool run_exception_test()
{
    //set_terminate(handler);
    foo foo1(0);
    try {
	foo foo2(1);
	{
	    foo bar(2);
	}
	{
	    foo bar(3);
	    cout << "to test interrupt, hit Ctrl+C" << endl;
	    sleep(4);
	    cout << "crashing!" << endl;
	    int* p = 0;
	    int b = *p;
	}
	return false;
    } catch(std::exception& e) {
	cout << "exception catched2: " << e.what() << endl;
	return true;
    }
}
int main()
{
    if( !run_exception_test() ) return 1;
    return UnitTest::RunAllTests();
}
