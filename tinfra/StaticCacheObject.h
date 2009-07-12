//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_StaticCacheObject_h_included
#define tinfra_StaticCacheObject_h_included


template <typename T>
	class StaticCacheObject {
		T instance;
		bool initialized;
		
		
	public:	
		
		StaticCacheObject() : initialized(false) {}
		bool isInitialized() const { return initialized; }
		bool setInitialized(bool i = true) { initialized = i; return i;}
				
		operator T&() { return instance; }
		operator const T&() const { return instance; }
		
		
		T& get() { return instance; }
		const T& get() const { return instance; }
	};

#endif
	
