//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/symbol.h"

#include <unittest++/UnitTest++.h>

#include "tinfra/mo.h"

namespace tinfra_mo_test {
    TINFRA_SYMBOL_IMPL(x);
    TINFRA_SYMBOL_IMPL(y);
    TINFRA_SYMBOL_IMPL(top_left);
    TINFRA_SYMBOL_IMPL(bottom_right);
    struct point {
        int x;
        int y;
        TINFRA_MO_MANIFEST(point) {
            TINFRA_MO_FIELD(x);
            TINFRA_MO_FIELD(y);
        }
    };
    
    struct rect {
        point top_left;
        point bottom_right;
        
        TINFRA_MO_MANIFEST(rect) {
            TINFRA_MO_FIELD(top_left);
            TINFRA_MO_FIELD(bottom_right);
        }
    };
    
    class point_bean {
        int x;
        int y;
        
    public:
        int getX() const { return x; }
        int getY() const { return y; }
        
        void setX(int v) { x = v; }
        void setY(int v) { y = v; }
        
        template <typename F>
        void apply(F& f) const
        {
            f(S::x, &point_bean::getX, &point_bean::setX);
            f(S::y, &point_bean::getY, &point_bean::setY);
        }
    };
}

namespace tinfra {
    template<> 
    struct mo_traits<tinfra_mo_test::point>: public tinfra::struct_mo_traits<tinfra_mo_test::point> {};
        
    template<> 
    struct mo_traits<tinfra_mo_test::rect>: public tinfra::struct_mo_traits<tinfra_mo_test::rect> {};
    
    template <typename MO, typename F>
    struct mo_bean_processor_adapter {
        MO const& mo;
        F&  f;
        template <typename T, typename X>
        void operator()(tinfra::symbol const& sym, T (MO::*getter)() const, X)
        {
            f(sym, (mo.*getter)());
        }
    };
    
    template <typename MO, typename F>
    struct mo_bean_mutator_adapter {
        MO& mo;
        F&  f;
        template <typename T>
        void operator()(tinfra::symbol const& sym, T (MO::*getter)() const, void (MO::*setter)(T const& v))
        {
            T orig = (mo.*getter)();
            T victim(orig);
            f(sym, victim);
            if( !(victim == orig) ) {
                mo.*setter(victim);
            }
        }
        
        template <typename T>
        void operator()(tinfra::symbol const& sym, T (MO::*getter)() const, void (MO::*setter)(T v))
        {
            T orig = (mo.*getter)();
            T victim(orig);
            f(sym, victim);
            if( !(victim == orig) ) {
                (mo.*setter)(victim);
            }
        }
    };
    
    template <typename F>
    void mo_process(tinfra_mo_test::point_bean const& v, F& f)
    {
        mo_bean_processor_adapter<tinfra_mo_test::point_bean,F> adapter = {v, f};
        v.apply(adapter);
    }
    
    template <typename F>
    void mo_mutate(tinfra_mo_test::point_bean& v, F& f)
    {
        mo_bean_mutator_adapter<tinfra_mo_test::point_bean,F> adapter = { v, f };
        v.apply(adapter);
    }
}

SUITE(tinfra)
{
    using tinfra_mo_test::point;
    using tinfra_mo_test::rect;
    using tinfra_mo_test::point_bean;
    using tinfra::symbol;
    
    struct dummy_functor {
        int sum;
        int count;
        void operator()(symbol const&, int const& v) {
            sum+=v;
            count++;
        }
        template <typename T>
        void mstruct(symbol const&, T const& v) {
            tinfra::mo_process(v, *this);
        }
    };
    
    TEST(mo_process_api)
    {
        dummy_functor f = {0,0};
        const point a = { 3,-2};
        tinfra::mo_process(a, f);
        CHECK_EQUAL(1, f.sum);
        CHECK_EQUAL(2, f.count);
    }
    
    TEST(mo_process_complex)
    {
        dummy_functor f = {0};
        const rect r = { { 3,-2} , {4, -3} };
        tinfra::mo_process(r, f);
        CHECK_EQUAL(2, f.sum);
        CHECK_EQUAL(4, f.count);
    }
    
    TEST(mo_process_bean_api)
    {
        dummy_functor f = {0};
        point_bean a;
        a.setX(1);
        a.setY(2);
        tinfra::mo_process(a, f);
        CHECK_EQUAL(3, f.sum);
    }
    
    struct foo_modifier {
        int count;
        void operator()(symbol const&, int& v ) { 
            v = 1; 
            count++;
        }
        
        template <typename T>
        void mstruct(symbol const&, T& v) {
            tinfra::mo_mutate(v, *this);
        }
    };
    
    TEST(mo_mutate_api)
    {
        foo_modifier f = {0};
        point a = { 0, 0 };
        tinfra::mo_mutate(a, f);
        
        CHECK_EQUAL(1, a.x);
        CHECK_EQUAL(1, a.y);
        CHECK_EQUAL(2, f.count);
    }
    
    TEST(mo_mutate_complex)
    {
        foo_modifier f = {0};
        rect r = { {0, 0}, {2,2} };
        tinfra::mo_mutate(r, f);
        
        CHECK_EQUAL(1, r.top_left.x);
        CHECK_EQUAL(1, r.top_left.y);
        CHECK_EQUAL(1, r.bottom_right.x);
        CHECK_EQUAL(1, r.bottom_right.y);
        CHECK_EQUAL(4, f.count);
    }
    
    TEST(mo_mutate_bean_api)
    {
        foo_modifier f;
        point_bean a;
        a.setX(0);
        a.setY(2);
        
        tinfra::mo_mutate(a, f);
        
        CHECK_EQUAL(1, a.getX());
        CHECK_EQUAL(1, a.getY());
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
