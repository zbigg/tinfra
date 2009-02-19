#include <tinfra/multitype_map.h>
#include <stdexcept>
#include <iostream>

#define LOG(w, e) std::cerr << #w ": " << e << std::endl;

template <typename IMPL, typename S>
class handler_map {    
public:
    
    handler_map(IMPL& i): impl(i) {}
    
    bool invoke(S const& state) {
        typedef void (IMPL::*function_type)();
        function_type fff = mmm.get(state, function_type(0));
        if( fff == 0 ) 
            return false;
        (impl.*fff)();
        return true;
    }
    
    template <typename P1>
    bool invoke(S const& state, P1 const& p1) {
        typedef void (IMPL::*function_type)(P1 const&);
        
        function_type fff = mmm.get(state, function_type(0));
        if( fff == 0 ) 
            return false;
        (impl.*fff)(p1);
        return true;
    }
    
    template <typename P1, typename P2>
    bool invoke(S const& state, P1 const& p1, P2 const& p2) {
        typedef void (IMPL::*function_type)(P1 const&, P2 const&);
        
        function_type fff = mmm.get(state, function_type(0));
        if( fff == 0 ) 
            return false;
        (impl.*fff)(p1,p2);
        return true;
    }
    
    void connect( S const& state, void (IMPL::*method)() ) {
        mmm.put(state, method);
    }
    
    template <typename P1>
    void connect( S const& state, void (IMPL::*method)(P1 const&)) {
        mmm.put(state, method);
    }
    
    template <typename P1, typename P2>
    void connect( S const& state, void (IMPL::*method)(P1 const&, P2 const&)) {
        mmm.put(state, method);
    }
    
private:
    tinfra::multitype_map<S> mmm;
    IMPL& impl;
};


template <typename IMPL, typename S>
class state_machine_base {
public:
    state_machine_base(IMPL& i,S const& initial_state) : 
        state_(initial_state),
        handler(i)
    {
        entering_state(initial_state);
    }
    virtual ~state_machine_base() {}
    
    S state() const { 
        return state_; 
    }
    
    void state(S const& s) {
        if( state_ != s ) {
            leaving_state(state_);
            state_ = s;
            entering_state(s);
        }
    }
    
    bool operator()() {
       return handler.invoke(state_);        
    }
    
    template <typename P1>
    bool operator()(P1 const& p1) {
        return handler.invoke(state_, p1);
    }
    
    template <typename P1, typename P2>
    bool operator()(P1 const& p1, P2 const& p2) {
        return handler.invoke(state_, p1, p2);
    }
    
    void check_complete()
    {
        // TODO: 
        // for all registered states
        //   for all regitered signal types
        //    check that there is resolution
        //
        // must be done in handler_map because uit owns
        // map of (state,type) -> handler
    }
private:
    S state_;
protected:
    virtual void leaving_state(S old_state)
    {
        LOG(state_machine_base::leaving_state, old_state);
    }
    virtual void entering_state(S new_state)
    {
        LOG(state_machine_base::entering_state, new_state);
    }
    handler_map<IMPL, S> handler;
};

enum states {
    MSM_IDLE,
    MSM_SLEEPING
};

class akuku: public state_machine_base<akuku, states> {
public:
    akuku() : 
        state_machine_base<akuku, states> (*this, MSM_IDLE),
        sleep_counter(0)
    {
        handler.connect(MSM_IDLE, &akuku::idle_message);
        handler.connect(MSM_SLEEPING, &akuku::sleep_message);
    }
    
    void idle_message()
    {
        LOG(akuku, "idle: message");
        state(MSM_SLEEPING);
        sleep_counter = 0;
    }
    
    int sleep_counter;
    void sleep_message()
    {
        
        LOG(akuku, "sleep: message");
        sleep_counter += 1;
        if( sleep_counter == 5 )
            state(MSM_IDLE);
    }
};


int main()
{
    akuku msm;
    msm.check_complete();
    for( int i = 0; i < 100; ++i )
        msm();
}
