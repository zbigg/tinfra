//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra__generic_factory_h__
#define __tinfra__generic_factory_h__

namespace tinfra {

//
// generic factory:
// usage:
//    struct interface_type {
//        typedef generic_factory_baseN<interface_type, P1, P2 ... PN> factory_type;
//    }
//
//    struct implementation {
//        implementation(P1, P2 ... PN)
//    }
//    typedef generic_factory_implN<implementation, interface_type::factory_type> factory_type;
//

template <typename T, typename P1, typename P2>
struct generic_factory_base2 {
	typedef T  interface_type;
	typedef P1 param1_type;
	typedef P2 param2_type;
	
	virtual T* create(P1, P2) = 0;
};

template <typename I, typename FT>
struct generic_factory_impl2: public FT {
	virtual typename FT::interface_type* create(typename FT::param1_type p1, 
		                                    typename FT::param2_type p2) {
		return new I(p1,p2);
	}
};

} // end namespace tinfra

#endif // __tinfra__generic_factory_h__

