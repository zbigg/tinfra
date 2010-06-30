//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/symbol.h"

#include <unittest++/UnitTest++.h>

#include "tinfra/mo.h"

struct true_type { enum { value = 1 } ; };

struct false_type { enum { value = 1 } ; };

namespace tinfra_mo_test {
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
    
    struct point3d {
        typedef true_type tinfra_is_mo;
        int x;
        int y;
        int z;
        TINFRA_MO_MANIFEST(point) {
            TINFRA_MO_FIELD(x);
            TINFRA_MO_FIELD(y);
            TINFRA_MO_FIELD(z);
        }
    };
    
    struct rect {
        point top_left;
        point bottom_right;
        
        TINFRA_MO_MANIFEST(rect) {
            TINFRA_MO_SYMBOL_FIELD(top_left);
            TINFRA_MO_SYMBOL_FIELD(bottom_right);
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
            f.leaf("x", &point_bean::getX, &point_bean::setX);
            f.leaf("y", &point_bean::getY, &point_bean::setY);
        }
    };
}

namespace tinfra {
    template<> 
    struct mo_traits<tinfra_mo_test::point>: public tinfra::struct_mo_traits<tinfra_mo_test::point> {};
        
    template<> 
    struct mo_traits<tinfra_mo_test::rect>: public tinfra::struct_mo_traits<tinfra_mo_test::rect> {};
        
        template<> 
    struct mo_traits<tinfra_mo_test::point_bean>: public tinfra::struct_mo_traits<tinfra_mo_test::point_bean> {};
    
    template <typename MO, typename F>
    struct mo_bean_processor_adapter {
        MO const& mo;
        F&  f;
        template <typename S, typename T, typename X>
        void leaf(S const& sym, T (MO::*getter)() const, X)
        {
            f.leaf(sym, (mo.*getter)());
        }
    };
    
    template <typename MO, typename F>
    struct mo_bean_mutator_adapter {
        MO& mo;
        F&  f;
        template <typename S, typename T>
        void leaf(S const& sym, T (MO::*getter)() const, void (MO::*setter)(T const& v))
        {
            T orig = (mo.*getter)();
            T victim(orig);
            f.leaf(sym, victim);
            if( !(victim == orig) ) {
                mo.*setter(victim);
            }
        }
        
        template <typename S, typename T>
        void leaf(S const& sym, T (MO::*getter)() const, void (MO::*setter)(T v))
        {
            T orig = (mo.*getter)();
            T victim(orig);
            f.leaf(sym, victim);
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
        void leaf(const char*, int const& v) {
            sum+=v;
            count++;
        }
        template <typename T>
        void record(const char*, T const& v) {
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
        int leaf_count;
        int struct_count;
        void leaf(const char*, int& v ) { 
            v = 1; 
            leaf_count++;
        }
        
        
        template <typename T>
        void record(const char*, T& v) {
            struct_count++;
            tinfra::mo_mutate(v, *this);
        }
        
        
    };
    
    TEST(mo_mutate_api)
    {
        foo_modifier f = {0,0};
        point a = { 0, 0 };
        tinfra::mutate("a", a, f);
        
        CHECK_EQUAL(1, a.x);
        CHECK_EQUAL(1, a.y);
        CHECK_EQUAL(2, f.leaf_count);
        CHECK_EQUAL(1, f.struct_count);
    }
    
    TEST(mo_mutate_complex)
    {
        foo_modifier f = {0,0};
        rect r = { {0, 0}, {2,2} };
        tinfra::mutate("r", r, f);
        
        CHECK_EQUAL(1, r.top_left.x);
        CHECK_EQUAL(1, r.top_left.y);
        CHECK_EQUAL(1, r.bottom_right.x);
        CHECK_EQUAL(1, r.bottom_right.y);
        CHECK_EQUAL(4, f.leaf_count);
        CHECK_EQUAL(3, f.struct_count);
    }
    
    TEST(mo_mutate_bean_api)
    {
        foo_modifier f = {0,0};
        point_bean a;
        a.setX(0);
        a.setY(2);
        
        tinfra::mutate("a", a, f);
        
        CHECK_EQUAL(1, a.getX());
        CHECK_EQUAL(1, a.getY());
        CHECK_EQUAL(2, f.leaf_count);
        CHECK_EQUAL(1, f.struct_count);
    }
    
    struct sfinae_functor {
        int sum;
        void leaf(const char*, int const& v ) {
            sum += v;
        }
        
        bool sfinae_matched;
        template <typename T>
        void leaf(const char*, T const& v, typename T::tinfra_is_mo = true_type()) {
            sfinae_matched = true;
            tinfra::mo_process(v, *this);
        }
    };
    
    TEST(mo_sfinae_processor)
    {
        sfinae_functor functor = {0, false};
        const tinfra_mo_test::point3d foo = { 1,2,3 };
        
        tinfra::process("foo", foo, functor);
        
        CHECK( functor.sfinae_matched);
        
        CHECK_EQUAL(6, functor.sum); 
    }
    
    struct sfinae_mutator {
        void leaf(const char*, int& v ) {
            v = 0;
        }
        
        bool sfinae_matched;
        template <typename T>
        void leaf(const char*, T& v, typename T::tinfra_is_mo = true_type()) {
            sfinae_matched = true;
            //v.foo =2; uncomment to see how unreadable error is
            tinfra::mo_mutate(v, *this);
        }
    };
    
    TEST(mo_sfinae_mutator)
    {
        sfinae_mutator functor = {false};
        tinfra_mo_test::point3d foo = { 1,2,3 };
        
        tinfra::mutate("foo", foo, functor);
        
        CHECK( functor.sfinae_matched);
        
        CHECK_EQUAL(0, foo.x);
        CHECK_EQUAL(0, foo.y);
        CHECK_EQUAL(0, foo.z);
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
