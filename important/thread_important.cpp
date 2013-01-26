#include <boost/thread.hpp>

#include "tinfra_important.h"

/*
static pthread_key_t key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;


static 
void make_key()
{
  (void) pthread_key_create(&key, NULL);
}


void get_()
{
  void *ptr;


  
  if ((ptr = pthread_getspecific(key)) == NULL) {
      ptr = malloc(OBJECT_SIZE);
      ...
      (void) pthread_setspecific(key, ptr);
  }
  ...
}
*/


__thread thread_stack_control_data thread_stack_control_data_inst = {0};

thread_stack_control_data* get_thread_stack_control_data()
{
	return &thread_stack_control_data_inst;
	
}

static void write_str(int fd, const char* str)
{
	const size_t len =  strlen(str);
	if( len != 0 ) {
		::write(fd, str, len);
	}
}
void tinfra_important::dumpCurrentStack(const char* prefix, int fd)
{
	thread_stack_control_data* cd = get_thread_stack_control_data();
	if( !cd )
		return;
	stack_var_entry* current = cd->top;
	char buf[1024];
	while( current ) {
		sprintf(buf, "%s%s:%i [%s]: %s = ", 
			prefix, 
			current->location.file, 
			(int)current->location.line, 
			current->location.func, 
			current->name);
		write_str(fd, buf);
		current->printer(fd, current->object);
		write_str(fd, "\n");
		
		current = current->prev;
	};
	
}

struct ThreadBar
{
	std::string task;
	void operator()()
	{
		TINFRA_IMPORTANT_NAME("ThreadBar");
		sleep(2);
	}
};

struct ThreadFoo
{
	std::string task;
	void operator()()
	{
		TINFRA_IMPORTANT_NAME("ThreadFoo");
		TINFRA_IMPORTANT(task);
		
		sleep(1);
		
		char* x = 0;
		*x = 1;
	}
};

int main(int argc, const char **argv)
{
	TINFRA_IMPORTANT_NAME("main");
	
	volatile	int *x=0;
	ThreadBar bar = {"barka"};
	ThreadFoo foo = {"foobar"};
	
	boost::thread thrd1(bar);
	boost::thread thrd2(foo);
	thrd2.join();
	thrd1.join();
	return 0;
}

