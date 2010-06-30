//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/symbol.h"

#include <unittest++/UnitTest++.h>

#include "tinfra/mo.h"

//struct true_type { enum { value = 1 } ; };
//struct false_type { enum { value = 1 } ; };

namespace tinfra_mo_test {    
    struct point {
        int x;
        int y;
        TINFRA_MO_MANIFEST {
            TINFRA_MO_NAMED_FIELD(x);
            TINFRA_MO_NAMED_FIELD(y);
        }
    };
    
    struct point3d {
        //typedef true_type tinfra_is_mo;
        int x;
        int y;
        int z;
        TINFRA_MO_MANIFEST {
            TINFRA_MO_NAMED_FIELD(x);
            TINFRA_MO_NAMED_FIELD(y);
            TINFRA_MO_NAMED_FIELD(z);
        }
    };
    
    struct rect {
        point top_left;
        point bottom_right;
        
        TINFRA_MO_MANIFEST {
            TINFRA_MO_NAMED_FIELD(top_left);
            TINFRA_MO_NAMED_FIELD(bottom_right);
        }
    };
    
    /* mo-beans disabled for a while */
    /*
    template <typename MO, typename T, typename TS>
    struct bean_property_info {
    public:
        typedef MO mo_type ;
        typedef T  value_type;
        
        typedef T    (MO::*getter_type)() const;
        typedef void (MO::*setter_type)(TS v);
    
        const char* const  name;
        getter_type const  getter;
        setter_type const  setter;
    };
    
    
    template <typename MO, typename T>
    bean_property_info<MO, T, T> make_bean_property_info(const char* name, T (MO::*getter)() const, void (MO::*setter)(T v))
    {
        const bean_property_info<MO, T, T> bpi = { name, getter, setter };
        return bpi;
    }
    
    template <typename MO, typename T>
    bean_property_info<MO, T, T const&> make_bean_property_info(const char* name, T (MO::*getter)() const, void (MO::*setter)(T const& v))
    {
        const bean_property_info<MO, T, T const&> bpi = { name, getter, setter };
        return bpi;
    }
    
    
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
            f(make_bean_property_info("x", &point_bean::getX, &point_bean::setX));
            f(make_bean_property_info("y", &point_bean::getY, &point_bean::setY));
        }
    };
    */
}

/*
namespace tinfra {
    template<> 
    struct mo_traits<tinfra_mo_test::point>: public tinfra::struct_mo_traits<tinfra_mo_test::point> {};
        
    template<> 
    struct mo_traits<tinfra_mo_test::rect>: public tinfra::struct_mo_traits<tinfra_mo_test::rect> {};
    
    template <typename MO, typename F>
    struct mo_bean_processor_adapter {
        MO const& mo;
        F&  functor;
        template <typename T, typename TS>
        void operator()(bean_property_info<MO, T, TS> const& bpi)
        {
            const T& value = mo.*(bpi.getter)();
            f(make_mo_named_value<T const&>(bpi.name, value));
        }
    };
    
    template <typename MO, typename F>
    struct mo_bean_mutator_adapter {
        MO& mo;
        F&  functor;
        template <typename T, typename TS>
        void operator()(bean_property_info<MO, T, TS> const& bpi)
        {
            const T& orig = mo.*(bpi.getter)();
            T victim(orig);
            functor(make_mo_named_value<T&>(bpi.name, victim);
            if( !(victim == orig) ) {
                mo.*(bpi.setter)(victim);
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

*/
SUITE(tinfra)
{
    using tinfra_mo_test::point;
    using tinfra_mo_test::rect;
    //using tinfra_mo_test::point_bean;
    
    struct dummy_functor {
        int sum;
        int count;
        
        
        void operator()(tinfra::mo_named_value<int const&>& v) {
            sum+=v.value;
            count++;
        }
        template <typename T>
        void mstruct(T const& v) {
            tinfra::mo_process(v, *this);
        }
        
        
        template <typename T> 
        void mstruct(tinfra::mo_named_value<T const&>& nv) {
            mstruct(nv.value);
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
    /*
    TEST(mo_process_bean_api)
    {
        dummy_functor f = {0};
        point_bean a;
        a.setX(1);
        a.setY(2);
        tinfra::mo_process(a, f);
        CHECK_EQUAL(3, f.sum);
    }
    */
    struct foo_modifier {
        int count;
        void operator()(int& v ) { 
            v = 1; 
            count++;
        }
        
        template <typename T>
        void mstruct(T& v) {
            tinfra::mo_mutate(v, *this);
        }
        
        // ignore names, when dealing with named MOs
        template <typename T> 
        void operator()(tinfra::mo_named_value<T> nv) {
            (*this)(nv.value);
        }
        template <typename T> 
        void mstruct(tinfra::mo_named_value<T> nv) {
            mstruct(nv.value);
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
    
    /*
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
    */
    
    //    NO SFINAE support for now
    /*
    struct sfinae_functor {
        int sum;
        void operator()(symbol const&, int const& v ) {
            sum += v;
        }
        
        bool sfinae_matched;
        template <typename T>
        void operator()(symbol const&, T const& v, typename T::tinfra_is_mo = true_type()) {
            sfinae_matched = true;
            tinfra::mo_process(v, *this);
        }
    };
    
    TEST(mo_sfinae_processor)
    {
        sfinae_functor functor = {0, false};
        const tinfra_mo_test::point3d foo = { 1,2,3 };
        
        tinfra::process(tinfra::symbol("a"), foo, functor);
        
        CHECK( functor.sfinae_matched);
        
        CHECK_EQUAL(6, functor.sum); 
    }
    
    struct sfinae_mutator {
        void operator()(symbol const&, int& v ) {
            v = 0;
        }
        
        bool sfinae_matched;
        template <typename T>
        void operator()(symbol const&, T& v, typename T::tinfra_is_mo = true_type()) {
            sfinae_matched = true;
            //v.foo =2; uncomment to see how unreadable error is
            tinfra::mo_mutate(v, *this);
        }
    };
    
    TEST(mo_sfinae_mutator)
    {
        sfinae_mutator functor = {false};
        tinfra_mo_test::point3d foo = { 1,2,3 };
        
        tinfra::mutate(tinfra::symbol("a"), foo, functor);
        
        CHECK( functor.sfinae_matched);
        
        CHECK_EQUAL(0, foo.x);
        CHECK_EQUAL(0, foo.y);
        CHECK_EQUAL(0, foo.z);
    }
    */
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
