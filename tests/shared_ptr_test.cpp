//
// Copyright (C) 2009 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/shared_ptr.h"
#include "tinfra/thread.h"

#include <unittest++/UnitTest++.h>

SUITE(tinfra) {
    using tinfra::shared_ptr;
    
    struct X
    {
        shared_ptr<X> next;
    };

    TEST(shared_ptr_implicit_reference)
    {
        shared_ptr<X> p(new X);
        p->next = shared_ptr<X>(new X);
        p = p->next;
        //CHECK(false);
        CHECK( p->next.get() == 0 );
    }
    
    
    void the_swap(shared_ptr<X>& a, shared_ptr<X>& b)
    {
        using std::swap;
        TINFRA_TRACE_MSG("hello");
        swap(a,b);
    }
    TEST(shared_ptr_swap)
    {
        shared_ptr<X> p(new X);
        shared_ptr<X> a;
        
        the_swap(a,p);
    }
    // TODO: this test rquires helgrind for statuc/dynamic
    // analysis of thread interlavinga

    void* fun(void* _a)
    {
	shared_ptr<int>* a = reinterpret_cast<shared_ptr<int>* >(_a);

	*( a->get()) = 2;
	delete a;
	return 0;
    }
    TEST(shared_ptr_thread_passing)
    {
	tinfra::thread::thread_set ts;
	shared_ptr<int> x;
	{
	    shared_ptr<int> a(new int(6));

	    shared_ptr<int>* b = new shared_ptr<int>(a);
	    ts.start(&fun, b);
	    x = a;
	}

	ts.join();
	CHECK_EQUAL(2, * (x.get()) );
    }
}
