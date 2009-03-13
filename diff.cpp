#include <tinfra/symbol.h>
#include <tinfra/cmd.h>

#include <iostream>
#include <tinfra/tinfra.h>

#define TINFRA_MANAGED_STRUCT(a)  template <typename F> void apply(F& f) const

#define TINFRA_MS_FIELD(a)    f(S::a, a)
    
#define TINFRA_SYMBOL_DECL(a) namespace S { extern tinfra::symbol a; }

#define TINFRA_SYMBOL_IMPL(a) tinfra::symbol S::a(#a)
TINFRA_SYMBOL_DECL(street);
TINFRA_SYMBOL_DECL(home);
TINFRA_SYMBOL_DECL(apartment);
TINFRA_SYMBOL_DECL(city);

struct address {
    std::string street;
    std::string home;
    std::string apartment;
    std::string city;
    
    TINFRA_MANAGED_STRUCT(address)
    {
        TINFRA_MS_FIELD(street);
        TINFRA_MS_FIELD(home);
        TINFRA_MS_FIELD(apartment);
        TINFRA_MS_FIELD(city);
    }
};

TINFRA_SYMBOL_IMPL(street);
TINFRA_SYMBOL_IMPL(home);
TINFRA_SYMBOL_IMPL(apartment);
TINFRA_SYMBOL_IMPL(city);

template <typename T>
T const& ref_in_other(const void* base_a, const T* value_in_a, const void* base_b)
{
    const char* cp_base_a = reinterpret_cast<const char*>(base_a);
    const char* cp_value_in_a = reinterpret_cast<const char*>(value_in_a);
    const char* cp_base_b = reinterpret_cast<const char*>(base_b);
    
    ptrdiff_t offset = (cp_value_in_a - cp_base_a);
    
    const char* cp_value_in_b = cp_base_b + offset;
    
    return reinterpret_cast<T const&>( *cp_value_in_b );
}

using tinfra::symbol;

class comparator {
    const void* a;
    const void* b;
public:
    comparator(const void* pa, const void* pb): a(pa), b(pb) {}

    template <typename V>
    void operator()(symbol const& sym, V const& va)
    {
        V const& vb = ref_in_other(a, &va, b);
        this->operator()(sym, va, vb);
    }
    
    template <typename V>
    void operator()(symbol const& sym, V const& va, V const& vb)
    {
        if( ! (va == vb ) ) {
            std::cout << sym << " different: '" << va << "' != '" << vb << "'\n";
        } else {
            std::cout << sym << " identical\n";
        }
    }
};
template <typename T>
void compare(T const& a, T const& b)
{
    comparator c(&a,&b);
    tinfra::process(a,c);
}


int diff_main(int argc, char** argv)
{
    address grzesiek = { "szczecinska", "39b", "", "Tanowo" };
    address mama = { "XXX-lecia", "12", "", "Tanowo" };
    //address ja = { "strzelecka", "5g", "11", "Szczecin" };
    //address sasiad = { "strzelecka", "5g", "12", "Szczecin" };
    //address tesciu = { "bandurskiego", "51", "2", "Szczecin" };
    
    compare(grzesiek, mama);
    return 0;
}

TINFRA_MAIN(diff_main);
