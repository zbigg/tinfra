#ifndef tinfra_important_h_included
#define tinfra_important_h_included
//
// TINFRA_IMPORTANT interface
//
#define TINFRA_IMPORTANT(var) 
    tinfra_important foo ## var(&var, #var, make_printer(var), __FILE__, __LINE__, __func__)
#define TINFRA_IMPORTANT_NAME(string) 
    tinfra_important foo_name(string, (const char*)"name", make_printer((const char*)string), __FILE__, __LINE__, __func__)

//
// TINFRA_IMPORTANT ABI - binary interface
//
struct source_code_location {
	const char* file;
	int         line;
	const char* func;
};

typedef void (*printer_func)(int fd, const void* object);
	
struct stack_var_entry {
	stack_var_entry* prev;
	
	source_code_location location;
	const char*   name;
	const void*   object;
	printer_func  printer;
};

struct thread_stack_control_data {
	stack_var_entry* top;
};

template <typename T>
void debug_print(int fd, T const* foo);

void debug_print(int fd, const char* foo)
{
	size_t len =  strlen(foo);
	::write(fd, foo, len);
}

void debug_print(int fd, std::string const* foo)
{
	//size_t len =  strlen(foo);
	::write(fd, foo->data(), foo->size());
}

template <typename T>
struct basic_printer {
	static void print(int fd, const void* obj)
	{
		T const* obj2 = reinterpret_cast<T const*>(obj);
		::debug_print(fd, obj2);
	}
};

template <>
struct basic_printer<const char*> {
	static void print(int fd, const void* obj)
	{
		::debug_print(fd, (const char*)obj);
	}
};

template <typename T>
printer_func make_printer(T const&)
{
	return &basic_printer<T>::print;
}

struct tinfra_important {
	tinfra_important(const void* data, const char* name, printer_func printer, const char* file, int line,const char* func);
	~tinfra_important();
	
	static void dumpCurrentStack(const char* prefix, int fd);
private:
	thread_stack_control_data* thread_local_control_data;
	stack_var_entry se;
};

thread_stack_control_data* get_thread_stack_control_data();

// 
// tinfra_important implementation
//

inline tinfra_important::tinfra_important(const void* data, const char* name, printer_func printer, const char* file, int line,const char* func)
{
	thread_stack_control_data* control_data = get_thread_stack_control_data();
	se.prev = control_data->top;
	se.location.file = file;
	se.location.line = line;
	se.location.func = func;
	se.name = name;
	se.object = data;
	se.printer = printer;
	control_data->top = &se;
}

inline tinfra_important::~tinfra_important()
{
	thread_stack_control_data* control_data = get_thread_stack_control_data();
	control_data->top = se.prev;
}


#endif // tinfra_important_h_included

