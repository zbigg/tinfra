//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
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
	
