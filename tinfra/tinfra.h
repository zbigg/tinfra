// tinfra.h

#ifndef __tinfra_h__
#define __tinfra_h__

#include "Symbol.h"
#include "StaticCacheObject.h"
#include <exception>

namespace tinfra {

template <typename T, typename A>
void process(const T& t, A& processor)
{
	t.apply(processor);
}
namespace detail {
	template <typename A>
	struct MutatorWrapper {
		A& mutator;
		MutatorWrapper(A& mutator) : mutator(mutator) {}
		
		template <class T>
			void operator () (const Symbol& s, const T& t) {
				mutator(s, const_cast<T&>(t));
			}
	};
}	
template <typename T, typename A>
void mutate(T& t, A& mutator)
{
	detail::MutatorWrapper<A> mutator_wrapper(mutator);
	process(const_cast<const T&>(t),mutator_wrapper);
}
	


namespace detail {

template<typename T>	
struct FieldCounter {
	int hitCount;
	int missCount;
	FieldCounter(): hitCount(0), missCount(0) {}

	// operations
	void operator() (const Symbol& s, const T& a) { hitCount++; }
	
	template <typename Other>
	void operator() (const Symbol& s, const Other& a) { missCount++; }
};
struct DummyType {};
    
typedef std::map<Symbol::id_type, int> OffsetMap;

template<typename T,typename V>
struct OffsetAnalyzer {
	OffsetMap& offsetMap;
	const T& parent;
	OffsetAnalyzer(const T& parent, OffsetMap& offsetMap) : parent(parent), offsetMap(offsetMap) {}
		
        // register only fields of type V
	void operator() (const Symbol& s, const V& a)
        {
            offsetMap[s] = (const char*)&a - (const char*)&parent;
        }
            
        // ignore rest
        template <typename boo>
        void operator() (const Symbol& s, const boo& a)
        {
        }
};


template <typename T,typename V>
const OffsetMap& getOffsetMap()
{
	static StaticCacheObject<OffsetMap> offsetMap;	
	
	if( !offsetMap.isInitialized() ) {
		T dummy;
		OffsetAnalyzer<T,V> analyzer(dummy, offsetMap);
		process(dummy, analyzer);
		offsetMap.setInitialized();
	}
	return offsetMap;
};

template<typename T>
struct SymbolsGetter {
	vector<Symbol>& dest;
	
	SymbolsGetter(vector<Symbol>& _dest) : dest(_dest) {}
		
        // register only fields of type V
        template <typename ANY>
	void operator() (const Symbol& s, const ANY& a)
        {
            dest.push_back(s);
        }
};

}

template<typename C>
int countFields()
{
	detail::FieldCounter<detail::DummyType> counter;
	process(C(), counter);
	return counter.missCount;
}

template<typename C, typename T>
int countFields()
{
	detail::FieldCounter<T> counter;
	process(C(), counter);
	return counter.hitCount;
}

template <typename T>
const std::vector<Symbol>& getSymbols()
{
	static StaticCacheObject<std::vector<Symbol> > symbols;
	if( !symbols.isInitialized() ) {
		detail::SymbolsGetter<T> getter(symbols.get());
                T dummy;
		process(dummy, getter);
		symbols.setInitialized();
	}
	return symbols;
}

///
///
/// TypeTraits infra
///
///

enum CompositeType {
	STRUCT, LIST, MAP
};

template <typename T>
struct TypeTraitsGeneric
{
    static const char* name() {
        const type_info& ti = typeid(T);
        return ti.name();
    }
    
    static Symbol symbol() {
        static Symbol theSymbol(name());
        return theSymbol;
    }
};

template <typename T>
struct TypeTraits: public TypeTraitsGeneric<T> {
    template <typename F>
    static void process(const T& t, const Symbol& s, F& f) { f(s,t); }
    
    template <typename F>
    static void mutate(T& t, const Symbol& s, F& f) { f(s,t); }
};

template <typename T>
struct Fundamental: public TypeTraitsGeneric<T> {
    template <typename F>
    static void process(const T& t, const Symbol& s, F& f) { f(s,t); }
    
    template <typename F>
    static void mutate(T& t, const Symbol& s, F& f) { f(s,t); }    
};

template <typename F>
struct TypeTraitsProcessCaller {
    F& f;
    TypeTraitsProcessCaller(F& _f) : f(_f) {}
            
    template <typename T1>
    void operator() (const Symbol& s, const T1& t)  { TypeTraits<T1>::process(t,s,f); }
    
    template <typename T1>
    void operator() (const Symbol& s, T1& t)  { TypeTraits<T1>::mutate(t,s,f); }
};

template <typename T, typename F>
void tt_process(T const& obj, F& f)
{
    TypeTraitsProcessCaller<F> f1(f);
    process(obj, f1);
}

template <typename T, typename F>
void tt_mutate(T& obj, F& f)
{
    TypeTraitsProcessCaller<F> f1(f);
    mutate<T>(obj, f1);
}


template <typename T>
struct ManagedStruct: public TypeTraitsGeneric<T>  {
    template <typename F>
    static void process(const T& t, const Symbol& s, F& f) {
        f.managed_struct(t, s);        
    }
    template <typename F>
    static void mutate(T& t, const Symbol& s, F& f) {
        f.managed_struct(t, s);
    }
};
template <typename T>
struct STLContainer: public TypeTraitsGeneric<T>  {
    template <typename F>
    static void process(const T& t, const Symbol& s, F& f) {        
        f.list_container(t, s);
    }
    template <typename F>
    static void mutate(T& t, const Symbol& s, F& f) {
        f.list_container(t, s);
    }
};

///
/// setters & getters
///

class field_error: public std::exception {
    std::string _what;
public:
    field_error(const char* c, const char* f) {
        _what = string("field_error: ") + string(c) + string("::") + string(f) + " not found or incompatible type";
    }
    virtual ~field_error() throw() {}
        
    const char* what() const throw() {
        return _what.c_str();
    }
};

template<typename T, typename X>
void set(X& x, const Symbol& key, const T& value)
{
    const detail::OffsetMap& om = detail::getOffsetMap<X,T>();
    
    detail::OffsetMap::const_iterator i = om.find(key);
    if( i == om.end() ) throw field_error(TypeTraits<X>::name(), (const char*)key);
    T* target = (T*)((char*)&x + i->second);
    *target = value;
}

template<typename T, typename X>
const T& get(const X& x, const Symbol& key)
{
    const detail::OffsetMap& om = detail::getOffsetMap<X,T>();    
    detail::OffsetMap::const_iterator i = om.find(key);
    
    if( i == om.end() ) throw field_error(TypeTraits<X>::name(), (const char*)key);
    
    const T* target = (const T*)( (const char*)&x + i->second );
    return *target;
}


template<class T>
struct ManagedType {
	static int countFields() { return tinfra::countFields<T>(); }

	template<class F>
	static int countFields() { return tinfra::countFields<T,F>(); }
	
	template<class F>
	void set(const Symbol& s, const F& value) { return tinfra::set<F, T>((T&)*this, s, value); }
	
	template<class F>
	const F& get(const Symbol& s) const { return tinfra::get<F, T>((const T&)*this, s); }
	
	static const std::vector<Symbol>& getSymbols() { return ::tinfra::getSymbols<T>(); }
};

template <class T>
struct constructor {
    T value;
    
    constructor() {}
        
    template<typename V>
    constructor(const Symbol& s, V const& x): value() { field(s,x); }
    
    T& get() { return value; }
    
    operator T&() { return value; }
    
    template <typename V>
    constructor<T>& field(const Symbol& s,  V const& x) { 
        tinfra::set(value, s, x); 
        return *this; 
    }
    
    template <typename V>
    constructor<T>& operator() (const Symbol& s,  V const& x) { return field(s, x); }
};
    
template <typename T>
constructor<T> construct() {
    return constructor<T>();
}
    
template <typename T, typename V>
constructor<T> construct(const Symbol& s,  V const& x) {
    return constructor<T>(s,x);
} 

}

#endif
