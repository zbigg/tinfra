#include <iostream>
#include <tinfra/any.h>

class ValueHolderBase {
    virtual 
};
class ProcessStateManager {
    template <typename T>
    void input(const char* name, T const& value)
    {
        this->registerValue(name, tinfra::any::from_ref<T const>(value));
    }
    
    template <typename T>
    void variable(const char* name, T const& value)
    {
        this->registerValue(name, tinfra::any::from_ref<T const>(value));
    }
private:
    struct {
    }
};

void foo(char*)
{
    next_state
}

state_handler

int main(int argc, char** argv)
{
    Bus bus = Bus::getDefault(argc, argv);
    ProcessStateManager SM("my_process_name");
    SM.input("argc", argc);
    SM.input("argv", argv);
    
    int sum = 0;
    SM.variable("sum", sum);
    while( bus.hasNext() ) 
    {
        Bus::Event ev = bus.next();
        {
            ProcessStateManager::Lock smlock(SM);
            sum += 1;
        }
    }
    SM.result("sum", sum);
}
