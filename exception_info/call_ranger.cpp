#include "call_ranger.h"  // we implement this

#include <tinfra/fmt.h>   // for tinfra::tprintf
#include <typeinfo>

#ifdef __GNUC__
#define HAVE_THREAD_LOCAL_KEYWORD 1
#define TINFRA_THREAD_LOCAL_KEYWORD __thread
#else

#include <pthread.h>
#endif


namespace tinfra {

#if defined(HAVE_THREAD_LOCAL_KEYWORD) && defined(TINFRA_THREAD_LOCAL_KEYWORD)

TINFRA_THREAD_LOCAL_KEYWORD call_ranger* thread_top_call_ranger = {0};

call_ranger* call_ranger::get_thread_local()
{
    return thread_top_call_ranger;
}

void call_ranger::register_instance()
{
    this->previous = thread_top_call_ranger;
    thread_top_call_ranger = this;
}

void call_ranger::unregister_instance()
{
    thread_top_call_ranger = this->previous;
}

#else // fall back to generic pthread implementation of
      // thread local

static pthread_key_t  call_ranger_thread_data_key;
static pthread_once_t call_ranger_thread_data_once = PTHREAD_ONCE_INIT;


static  void call_ranger_thread_key_init()
{
    pthread_key_create(&call_ranger_thread_data_key, NULL);
}

call_ranger* call_ranger::get_thread_local()
{
    pthread_once(&call_ranger_thread_data_once, &call_ranger_thread_key_init);
    return reinterpret_cast<call_ranger*>(pthread_getspecific(call_ranger_thread_data_key));
}

void call_ranger::register_instance()
{
    this->previous = call_ranger::get_thread_local();
    pthread_setspecific(call_ranger_thread_data_key, this);
}

void call_ranger::unregister_instance()
{
    pthread_setspecific(call_ranger_thread_data_key, this->previous);
}

#endif

void call_ranger::push_variable(call_ranger_variable* var)
{
    var->next = this->variables;
    this->variables = var;
}

void call_ranger::inform_about_exceptional_leave(tinfra::output_stream& out)
{
    // TBD, it shall be safe ;)
    const char* exception_name = "?";
    tinfra::exception_info ei;
    if( tinfra::exception_info::get_current_exception(ei) ) {
        if( ei.exception_type )
            exception_name = ei.exception_type->name();
        
        if( exception_name == 0 )
            exception_name = "?";
    }
    tinfra::tprintf(out,
        "PREFIX: exception (%s) leave of function %s (%s:%i), key variables follow:\n",
        exception_name,
        this->source_location->name,
        this->source_location->filename,
        this->source_location->line);
    call_ranger_variable* current = this->variables;
    while( current ) {
        // dump variable in reverse-vs-declaration order
        
        tinfra::tprintf(out, "PREFIX:    %s = '", current->name);
        current->printer(out, current->object);
        out.write("'\n", 2);
        
        current = current->next;
    }
}

void call_ranger::dump_info(tinfra::output_stream& out, bool recursive)
{
    tinfra::tprintf(out,
        "function %s (%s:%i), key variables follow:\n",
        this->source_location->name,
        this->source_location->filename,
        this->source_location->line);
    call_ranger_variable* current = this->variables;
    while( current ) {
        // dump variable in reverse-vs-declaration order
        
        tinfra::tprintf(out, "    %s = '", current->name);
        current->printer(out, current->object);
        out.write("'\n", 2);
        current = current->next;
    }
    if( recursive && this->previous ) {
        this->previous->dump_info(out, true);
    }
}

} // end namespace tinfra

