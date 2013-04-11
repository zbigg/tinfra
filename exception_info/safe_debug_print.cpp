#include "safe_debug_print.h" // we implement this

#include <tinfra/fmt.h> // for tinfra::tprintf, TBD, to be removed


void tinfra_safe_debug_print(tinfra::output_stream& out, char  const* v) {
    tinfra::tprintf(out, "%s", (int)*v);
}
/*
void tinfra_safe_debug_print(tinfra::output_stream& out, signed char*) {
}

void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned char*) {
}

void tinfra_safe_debug_print(tinfra::output_stream& out, short*) {
}

void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned short*) {
}
*/
void tinfra_safe_debug_print(tinfra::output_stream& out, int const* v) {
    tinfra::tprintf(out, "%s", *v);
}

/*
void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned int*) {
}

void tinfra_safe_debug_print(tinfra::output_stream& out, long*) {
}

void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned long*) {
}

void tinfra_safe_debug_print(tinfra::output_stream& out, long long*) {
}

void tinfra_safe_debug_print(tinfra::output_stream& out, unsigned long long*) {
}
*/
void tinfra_safe_debug_print(tinfra::output_stream& out, const char** foo)
{
    // TBD, escape etc
	size_t len =  std::strlen(*foo);
	out.write(*foo, len);
}

void tinfra_safe_debug_print(tinfra::output_stream& out, std::string const* foo)
{
    // TBD, escape etc
	out.write(foo->data(), foo->size());
}
