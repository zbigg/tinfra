#include <map>
#include <string>
#include <typeinfo>
#include <iostream>
#include <sstream>

// catch clause analysis

struct __cxa_exception { 
    std::type_info *	exceptionType;
    void (*exceptionDestructor) (void *); 
    void*	unexpectedHandler;
    void*	terminateHandler;
    __cxa_exception *	nextException;

    int			handlerCount;
    int			handlerSwitchValue;
    const char *        actionRecord;
    const char *	languageSpecificData;
    void *		catchTemp;
    void *		adjustedPtr;

    void*	        unwindHeader;
};

struct __cxa_eh_globals {
    __cxa_exception *	caughtExceptions;
    unsigned int        uncaughtExceptions;
};

extern "C" __cxa_eh_globals* __cxa_get_globals();

struct extended_type_info {
    virtual const char* type_name() = 0;
    virtual std::string to_string(void*) = 0;
    
    virtual ~extended_type_info() {}
};

typedef std::map<std::string, extended_type_info*> handler_map;

template <typename Callable>
void execute(Callable c, handler_map const& exc_handlers)
{
    try {
        c();
    } catch( ... ) {
        __cxa_eh_globals* gp = __cxa_get_globals();
        
        if( gp == 0 && gp->caughtExceptions == 0 ) {
            std::cerr << "unknown exception caught (unable to find exception record)" << "\n";
        } else {
            __cxa_exception* cp = gp->caughtExceptions;
            std::type_info* t = cp->exceptionType;
            void* exc_obj_ptr = cp->adjustedPtr;
            std::string exc_name(t->name());
            handler_map::const_iterator ieh = exc_handlers.find(exc_name);
            if( ieh != exc_handlers.end() ) {
                extended_type_info* ti = ieh->second;
                std::cerr << "exception caught " << ti->type_name() << "(" << ti->to_string(exc_obj_ptr) << ")\n";
            } else {
                std::cerr << "exception caught: " << t->name() << ", no handler found\n";
            }                
        }
    }
}

struct int_extended_type_info: public extended_type_info  {
    const char* type_name() { return "int"; }
    std::string to_string(void* p) {
        int* v = reinterpret_cast<int*>(p);
        std::ostringstream ss;
        ss << *v;
        return ss.str();
    }
};

struct std_string_extended_type_info: public extended_type_info  {
    const char* type_name() { return "std::string"; }
    std::string to_string(void* p) {
        std::string* v = reinterpret_cast<std::string*>(p);
        return *v;
    }
};

template <typename T>
class singletonof {
public:
    static T* get_ptr() { return &instance; }
private:
    static T instance;
};

template <typename T>
T singletonof<T>::instance;

// the code

void akuku()
{
    throw 321;
}

void akuku_string()
{
    throw std::string("akuku");
}

int main(int argc, char** argv)
{
    handler_map hm;
    hm["i"] = singletonof<int_extended_type_info>::get_ptr();
    hm["Ss"] = singletonof<std_string_extended_type_info>::get_ptr();
    execute(&akuku, hm);
    execute(&akuku_string, hm);
}
