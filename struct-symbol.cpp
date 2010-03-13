#include <string>
#include <iostream>

namespace symbol {
    struct first_name  {};
    struct last_name   {};
    struct email {};
    struct id {};
        
    struct person_id {};
    struct car_id {};


    struct make {};
    struct production_year {};
}
    
struct person {
    std::string first_name;
    std::string last_name;
    std::string email;
    
    int         id;
};

struct car {
    std::string make;
    int         production_year;
    int         id;
    
    template <typename Functor>
    void process(Functor& f) const {
        f(symbol::make(), this->make);
        f(symbol::production_year(), this->production_year);
        f(symbol::id(), this->id);
    }
};

struct car_person_mapping {
    int         person_id;
    int         car_id;
};

template <typename M, typename V>
struct getter {
    V value;
    void operator()(M, V const& v) {
        value  = v;
    }
    template <typename Mx, typename Vx>
    void operator()(Mx, Vx const&) {}
};


template <typename M, typename V, typename S>
V akukupaniekruku(S const& v)
{
    getter<M,V> g;
    v.process(g);
    return g.value;
}

int moo(car const& c)
{
    return akukupaniekruku<symbol::production_year, int>(c);
}
int main()
{
    car A = { "ford", 1999, 34 };
    using namespace symbol;
    printf("aaaa\n");
    std::string m = akukupaniekruku<make, std::string>(A);
    printf("bbbb\n");
    int  y = akukupaniekruku<production_year, int>(A);
    printf("cccc\n");
    std::cout << m << " " << y << "\n";
}
    
    
