#include <iostream>
#include <typeinfo>
#include <tuple>

template <typename... Rest>
void list_type_names_impl(int i);

template <typename First, typename... Rest>
void list_type_names_impl( int i, First&& a, Rest&&... rest)
//void list_type_names_impl( int i)
{
	std::cout << i << " " << typeid(First).name() << "\n";
	list_type_names_impl<Rest...>(i+1, std::forward<Rest>(rest)...);
	//list_type_names_impl<Rest...>(i+1);
}

template <typename... Rest>
void list_type_names_impl(int i)
{
}

template <typename... Arguments>
void list_type_names(Arguments&&... args)
{	
	list_type_names_impl(0, std::forward<Arguments>(args)...);
	//list_type_names_impl<Arguments...>(0);
}

template <typename ... FOO> 
struct X{
};

//template <typename Head, typename ... Types>
//struct type_enumerator<X<Head,Types...>>;

template<std::size_t idx, typename... types>
struct type_enumerator; 

template<size_t idx>
struct type_enumerator<idx>
{ 
	static void print() {}
};

template <size_t idx, typename Head, typename ... Tail>
struct type_enumerator<idx, Head, Tail...> 
{
	static void print() {
		
		std::cout  << idx << " " << typeid(Head).name() << "\n";
		type_enumerator<idx+1, Tail...>::print();
	}
};

template <typename... T>
struct TE: public type_enumerator<0, T...> {};

int main()
{
	list_type_names<int, float, std::string>(1, 2.0, "32");
	TE<int, float, std::string, TE<int>>::print();
}
