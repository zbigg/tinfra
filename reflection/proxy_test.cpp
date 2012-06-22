#include "proxy.h"

#include <tinfra/cmd.h>
#include <tinfra/test.h>

#include <string>
#include <iostream>

struct foo_value {
    std::vector<int> a;
};

class bar_interface {
public:    
    virtual ~bar_interface() {}

    virtual int       method0() = 0;
    virtual foo_value method1(int,int) = 0;
    virtual int       method1x(int,int) = 0;
    virtual int       method2(int,int,int,int) = 0;

};

TINFRA_PROXY1_BEGIN(bar_interface) {
    TINFRA_PROXY1_METHOD(0, bar_interface, method0);
    TINFRA_PROXY1_METHOD(1, bar_interface, method1);
    TINFRA_PROXY1_METHOD(2, bar_interface, method1x);
    TINFRA_PROXY1_METHOD(3, bar_interface, method2);
}

using tinfra::any;
using std::vector;

class my_proxy_handler: public tinfra::proxy_handler {
public:
    virtual any invoke(std::string const& method, vector<any>& args) 
    {
        std::cout << "proxy called\n";
        std::cout << "method: " << method << "(" << args.size() << " arg count)\n";
        if( method == "method0" ) {
            return any::from_copy(1337);
        }
        else if( method == "method1" ) {
            CHECK_EQUAL(2, args.size());
            int v1 = args[0].get<int>();
            int v2 = args[1].get<int>();
            CHECK_EQUAL(666, v1);
            CHECK_EQUAL(-666, v2);
            foo_value result;
            result.a.push_back(666);
            result.a.push_back(777);
            return any::from_copy(result);
        } else if( method == "method2" ) {
            CHECK_EQUAL(4, args.size());
            int v1 = args[0].get<int>();
            int v2 = args[1].get<int>();
            int v3 = args[2].get<int>();
            int v4 = args[3].get<int>();
            std::cout << tinfra::tsprintf("args: %i %i %i %i\n", v1,v2,v3,v4);
            CHECK_EQUAL(666, v1);
            CHECK_EQUAL(-666, v2);
            CHECK_EQUAL(123456789, v3);
            CHECK_EQUAL(-123456789, v4);
            return any::from_copy(v1+v2+v3+v4);
        } else {
            assert(0);
        }
    }
};

class bar_interface_impl: public bar_interface {
public:    
    virtual int       method0();
    virtual foo_value method1(int,int);
    virtual int       method1x(int,int);
    virtual int       method2(int,int,int,int);

};

int       bar_interface_impl::method0() { return 0;}
foo_value bar_interface_impl::method1(int,int) { return foo_value(); }
int       bar_interface_impl::method1x(int,int) { return 0;}
int       bar_interface_impl::method2(int,int,int,int) { return 0; }

extern void call_method0(bar_interface* bar)
{
    int x = bar->method0();
}

extern void call_method1(bar_interface* bar)
{
    foo_value res = bar->method1(666,-666);
}

extern void call_method1x(bar_interface* bar)
{
    int res = bar->method1x(666,-666);
}

int xxx(int, char**)
{
    bar_interface_impl xff;

    
    my_proxy_handler handler;    
    tinfra::proxy_object<bar_interface> bar = tinfra::make_proxy<bar_interface>(handler);
    
    bar_interface* pbar = bar.get();
 
    std::cout << "ptr=" << sizeof(foo_value) << "\n";
    std::cout << "ptr=" << bar.get() << "\n";
    
    {
        CHECK_EQUAL(1337, bar->method0());
    }

    {
        int res = bar->method2(666,-666, 123456789, -123456789);
        CHECK_EQUAL(0, res);
    }

    std::cout << "ptr=" << bar.get()  << "\n";
    {
        foo_value res = bar->method1(666,-666);
        
        std::cout << "res_ptr=" << &res << "\n";
        
        CHECK_EQUAL(2, res.a.size());
        CHECK_EQUAL(666, res.a[0]);
        CHECK_EQUAL(777, res.a[1]);
    }
    
    
    return 0;
}
    
TINFRA_MAIN(xxx);

