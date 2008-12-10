#ifndef __tinfra_trace_h__
#define __tinfra_trace_h__

namespace tinfra {

#define TINFRA_TRACE(tracer, message) do { if( tracer.is_enabled() ) { tracer.trace(__FILE__, __LINE__, message); } while(false); 

class tracer {
    bool enabled;
    const char* name;
    
    tracer(const char* name): enabled(false), name(name) {}
public:
    
    virtual bool is_enabled() const { return enabled; }
    void         enable(bool e = true) { enabled = e; }
    
    const char*  get_name() const { return name; }
    
    virtual void trace(const char* file_name, int line, const char* message);
    
    virtual void trace(const char* file_name, int line, std::string const& message)
    {
        return trace(file_name, line, message.c_str());
    }
};

}

#endif
