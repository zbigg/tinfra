

#ifndef tinfra_callback_h_included__
#define tinfra_callback_h_included__

template <typename T>
struct callback_base {
	
	virtual void operator()(T const& v) = 0;
	virtual ~callback_base() {}
};

template <typename T>
struct function_callback: public callback_base<T> {
	
	typedef void (*function_type)(T const&);
	
	function_callback(function_type fun):
		fun_(fun) 
	{}
	
	void operator()(T const& v) 
	{
		fun_(v);
	}
private:
	function_type fun_;
};

template <typename T, typename IMPL>
struct member_callback: public callback_base<T> {
	typedef IMPL impl_class;
	typedef void (impl_class::*member_fun_type)(T const&);
	
	impl_class& impl_;
	member_fun_type fun_;
	
	member_callback(impl_class& impl, member_fun_type fun):
		impl_(impl),
		fun_(fun) 
	{}
	
	void operator()(T const& v) 
	{
		(impl_.*fun_)(v);
	}
};

template <typename T, typename IMPL>
struct functor_callback: public callback_base<T> {
	typedef IMPL impl_class;
	
	impl_class& impl_;
	
	functor_callback(impl_class& impl):
		impl_(impl)
	{}
	
	void operator()(T const& v) 
	{
		impl_(v);
	}
};

template <typename T>
struct callback {
	
	callback() 
	{}
	
	template <typename V>
	callback(V const&v)
		: ptr_(new V(v)) {}
		
	void operator()(T const& v) {
		(* ptr_.get() )(v);
	}
		
private:
	typedef callback_base<T> value_type;
	tinfra::shared_ptr<value_type> ptr_;
};

template <typename T, typename IMPL>
member_callback<T,IMPL> make_callback(IMPL& obj, void (IMPL::*method)(T const&))
{
	return member_callback<T,IMPL>(obj, method);
}

template <typename T>
function_callback<T> make_callback(void (*fun)(T const&))
{
	return function_callback<T>(fun);
}

template <typename T, typename IMPL>
functor_callback<T,IMPL> make_callback(IMPL& functor)
{
	return functor_callback<T,IMPL>(functor);
}

#endif


