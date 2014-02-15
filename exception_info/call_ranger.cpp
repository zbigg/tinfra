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

//
// call_ranger :: thread local storage
//
#if defined(HAVE_THREAD_LOCAL_KEYWORD) && defined(TINFRA_THREAD_LOCAL_KEYWORD)

TINFRA_THREAD_LOCAL_KEYWORD call_ranger_frame* thread_top_call_ranger_frame = {0};

call_ranger_frame* call_ranger::get_thread_local()
{
    return thread_top_call_ranger_frame;
}

void call_ranger::register_instance()
{
    this->frame.previous = thread_top_call_ranger_frame;
    if( this->callback == 0 ) {
        this->callback = get_default_callback();
    }
    thread_top_call_ranger_frame = &this->frame;
}

void call_ranger::unregister_instance()
{
    thread_top_call_ranger_frame = this->frame.previous;
}

#else // fall back to generic pthread implementation of
      // thread local

static pthread_key_t  call_ranger_thread_data_key;
static pthread_once_t call_ranger_thread_data_once = PTHREAD_ONCE_INIT;


static  void call_ranger_thread_key_init()
{
    pthread_key_create(&call_ranger_thread_data_key, NULL);
}

call_ranger_frame* call_ranger::get_thread_local()
{
    pthread_once(&call_ranger_thread_data_once, &call_ranger_thread_key_init);
    return reinterpret_cast<call_ranger*>(pthread_getspecific(call_ranger_thread_data_key));
}

void call_ranger::register_instance()
{
    this->frame.previous = call_ranger::get_thread_local();
    pthread_setspecific(call_ranger_thread_data_key, &this->frame);
}

void call_ranger::unregister_instance()
{
    pthread_setspecific(call_ranger_thread_data_key, this->frame.previous);
}

#endif

//
// call_ranger :: basic methods
//
call_ranger_frame* call_ranger::get_frame() 
{
    return &this->frame;
}
call_ranger_frame* call_ranger::get_previous_frame() const
{
    return this->frame.previous;
}

tinfra::source_location call_ranger::get_source_location() const
{
    return *this->frame.source_location;
}

const call_ranger_variable* call_ranger::get_variables() const
{
    return this->frame.variables;
}

void call_ranger::push_variable(call_ranger_variable* var)
{
    var->next = this->frame.variables;
    this->frame.variables = var;
}

//
// call_ranger_frame :: helper functions
//
void log_exceptional_leave(call_ranger_frame const& frame, tinfra::output_stream& out, bool recursive)
{
    // TBD, it shall be safe ;)
    const char* exception_name = "?";
    const char* exception_what = "?";
    tinfra::exception_info ei;
    if( tinfra::exception_info::get_current_exception(ei) ) {
        if( ei.exception_type ) {
            exception_name = ei.exception_type->name();
        }

        if( exception_name == 0 ) {
            exception_name = "?";
        }

        // TBD, exception_what = ...
    }
    tinfra::tprintf(out,
        "warning: exceptional leave of function: %s: %s\n",
        exception_name,
        exception_what
        );
    dump_call_info(frame, out, recursive);
}

static 
int call_ranger_frame_len(call_ranger_frame const* frame)
{
    int r = 0;
    while( frame ) {
        r += 1;
        frame = frame->previous;
    }
    return r;
}
void dump_call_info(call_ranger_frame const& frame, tinfra::output_stream& out, bool recursive)
{
    int depth = call_ranger_frame_len(&frame);
    call_ranger_frame const* crf = &frame;
    int i = depth;
    while( crf ) {
        // TBD, we shall use SAFE runtime
        // or ... guarantee that tprintf is SAFE
        tinfra::tprintf(out,
            "#%i: function %s (%s:%i), key variables follow:\n",
            i,
            crf->source_location->name,
            crf->source_location->filename,
            crf->source_location->line);
        // dump variable in reverse-vs-declaration order
        call_ranger_variable* current = crf->variables;
        while( current ) {
            // TBD, we shall use SAFE runtime
            // or ... guarantee that tprintf is SAFE
            tinfra::tprintf(out, "    %s = '", current->name);
            current->printer(out, current->object);
            out.write("'\n", 2);
            current = current->next;
        }
        if( !recursive )
            break;
        crf = crf->previous;
        i--;
    }
}

//
// call ranger :: default callback mechanism
//

static call_ranger_callback* call_ranger_default_callback_ptr = 0;

call_ranger_callback* call_ranger::get_default_callback()
{
    return call_ranger_default_callback_ptr;
}
void                  call_ranger::set_default_callback(call_ranger_callback* ncb)
{
    call_ranger_default_callback_ptr = ncb;
}

struct call_ranger_default_callback: public call_ranger_callback {
    virtual void exceptional_leave(call_ranger_frame& data)
    {
        log_exceptional_leave(data, tinfra::err, true);
    }
} call_ranger_default_callback_inst;

void                  call_ranger::init_default_callback()
{
    call_ranger_default_callback_ptr = &call_ranger_default_callback_inst;
}

} // end namespace tinfra

