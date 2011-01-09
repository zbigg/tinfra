#ifndef tinfra_type_visitor_h_included
#define tinfra_type_visitor_h_included

template<typename Functor, std::size_t idx, typename... types>
struct type_visitor_impl; 

template<typename Functor, size_t idx>
struct type_visitor_impl<Functor, idx>
{ 
	static void visit(Functor) {}
};

template <typename Functor, size_t idx, typename Head, typename ... Tail>
struct type_visitor_impl<Functor, idx, Head, Tail...> 
{
	static void visit(Functor f) {
		
		//std::cout  << idx << " " << typeid(Head).name() << "\n";
		f.template visit<Head>(idx);
		//xsd_for_param(fmt("arg%i")%idx, "xsd:int", xml);
		type_visitor_impl<Functor, idx+1, Tail...>::visit(f);
	}
};

template <typename Functor, typename ... Types>
void visit_types(Functor f)
{
	type_visitor_impl<Functor, 0, Types...>::visit(f);
}

#endif // tinfra_type_visitor_h_included

