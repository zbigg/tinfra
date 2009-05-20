#include <tinfra/multitype_map.h>
#include <tinfra/symbol.h>
#include <tinfra/thread.h>

#include <tinfra/trace.h>
#include <tinfra/cmd.h>

using tinfra::symbol;

symbol symbol_default("default");

class shared_container {
public:
	template <typename T>
	T get() {
		return get<T>(symbol_default);
	}
	
	template <typename T>
	T get(symbol const& what) {
		tinfra::thread::guard instance_guard(instance_lock);
		return themap.get<T>(what);
	}
	
	template <typename T>
	bool put(T const& original, T const& updated)
	{
		return put<T>(symbol_default, original, updated);
	}
	
	template <typename T>
	bool put(symbol what, T const& original, T const& updated)
	{
		tinfra::thread::guard instance_guard(instance_lock);
		
		T const& actual = themap.get<T>(what);
		
		if( original != actual ) {
			return false;
		}
		themap.put<T>(what, updated);
		return true;
	}
	
	template <typename T, typename R>
	bool update(symbol what, T const& original, T const& updated, R resolver)
	{
		if( put(what, original, updated ) ) 
			return true;
		
		T ours(updated);
		T current(get<T>(what));
		while (true ) {
			TINFRA_TRACE_MSG("update failed, trying to resolve conflict");
			TINFRA_TRACE_VAR(original);
			TINFRA_TRACE_VAR(current);
			TINFRA_TRACE_VAR(ours);
			if( !resolver(what, original, current, ours) ) {
				TINFRA_TRACE_MSG("resolver failed to merge");
				return false;
			}
			TINFRA_TRACE_MSG("resolver merged");
			TINFRA_TRACE_VAR(ours);
			
			if( put(what, current, ours) )
				return true;
			
			current = get<T>(what);
		}
	}
private:
	tinfra::multitype_map<symbol> themap;
	tinfra::thread::mutex         instance_lock;
};

template <typename K, typename V>
std::ostream& operator<<(std::ostream& s, std::vector< std::pair<K,V> > const& v)
{
	for(unsigned int i = 0; i < v.size(); ++i ) {
		if( i > 0 )
			s << ", ";
		s << "(" << v[i].first << "," << v[i].second << ")";
	}
	return s;
}

template <typename K, typename T>
std::vector< std::pair<K,T> > get_type_map(tinfra::multitype_map<K> const& vvv)
{
	typename std::map<K,T>::const_iterator b = vvv.template begin<T>();
	typename std::map<K,T>::const_iterator e = vvv.template end<T>();
	return std::vector< std::pair<K,T> >( b, e );
}
bool gui_conflict_handler(symbol sym, int original,  int theirs, int& yours_and_result)
{
	// here one must choice between
	// theirs and
	// yours_and_result and put result in
	// yours_and_result
	yours_and_result = theirs;
	return true;
}

int shared_container_test_main(int argc, char** argv)
{
	
	shared_container cont;
	int actual = cont.get<int>();
	TINFRA_TRACE_VAR(actual);
	bool result;
	result = cont.put<int>(actual,12);
	TINFRA_TRACE_VAR(result);
	TINFRA_TRACE_VAR(cont.get<int>());
	
	result = cont.put<int>(actual,13);
	TINFRA_TRACE_VAR(result);
	TINFRA_TRACE_VAR(cont.get<int>());
	
	cont.update<int>(symbol_default, actual, 13, gui_conflict_handler);
	TINFRA_TRACE_VAR(cont.get<int>());
	
	return 0;
}

TINFRA_MAIN(shared_container_test_main);

