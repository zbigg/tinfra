#ifndef __tinfra_exception_h_
#define __tinfra_exception_h_

namespace tinfra {

struct stackentry {
    void*	address;
    std::string symbol;
};

typedef std::vector<stackentry> stacktrace_t;

void populate_stacktrace(stacktrace_t& dest,int ignore_stacks=0);

} // end of namespace tinfra

#endif
