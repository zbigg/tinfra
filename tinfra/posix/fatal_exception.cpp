
///
/// fatal exception handling for posix-like systems
///
static void (*fatal_exception_handler) (void) = 0;

namespace tinfra {
    
void initialize_fatal_exception_handler(void (*handler) (void))
{    
    fatal_exception_handler = handler;
}

}

