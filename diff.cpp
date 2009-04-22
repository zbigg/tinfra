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


template <typename T>
struct mo: public T {
    typedef T  value_type;
    typedef T& reference;
    mo(T const& v): T(v) {}
};

struct address_data {
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

typedef mo<address_data> address;

TINFRA_SYMBOL_IMPL(street);
TINFRA_SYMBOL_IMPL(home);
TINFRA_SYMBOL_IMPL(apartment);
TINFRA_SYMBOL_IMPL(city);

template <typename T>
T& ref_in_other(const void* base_a, const T* value_in_a, void* base_b)
{
    const char* cp_base_a = reinterpret_cast<const char*>(base_a);
    const char* cp_value_in_a = reinterpret_cast<const char*>(value_in_a);
    char* cp_base_b = reinterpret_cast<char*>(base_b);
    
    ptrdiff_t offset = (cp_value_in_a - cp_base_a);
    
    char* cp_value_in_b = cp_base_b + offset;
    
    return reinterpret_cast<T&>( *cp_value_in_b );
}

template <typename T>
T const& const_ref_in_other(const void* base_a, const T* value_in_a, const void* base_b)
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
        V const& vb = const_ref_in_other(a, &va, b);
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

class swapper {
    void* a;
    void* b;
public:
    swapper(void* pa, void* pb): a(pa), b(pb) {}

    template <typename V>
    void operator()(symbol const& sym, V& va)
    {
        V & vb = ref_in_other(a, &va, b);
        this->operator()(sym, va, vb);
    }
    
    template <typename V>
    void operator()(symbol const& , V& va, V & vb)
    {
        swap(va, vb);
    }
};

template <typename T>
void tinfra_swap(T& a, T& b)
{
    swapper s(&a, &b);
    tinfra::mutate(a, s);
}

namespace std {
template <typename T>
void swap(mo<T>& a, mo<T>& b)
{
    std::cerr << "USED managed_type swap\n";
    //std::swap(a,b);
    tinfra_swap(a,b);
}
}

int diff_main(int, char**)
{
    address_data d1 = { "szczecinska", "39b", "", "Tanowo" };
    address_data d2 = { "szczecinska", "39b", "", "Tanowo" };
    address grzesiek(d1);
    address mama(d2);
    //address ja = { "strzelecka", "5g", "11", "Szczecin" };
    //address sasiad = { "strzelecka", "5g", "12", "Szczecin" };
    //address tesciu = { "bandurskiego", "51", "2", "Szczecin" };
    
    compare(grzesiek, mama);
    std::swap(grzesiek, mama);
    compare(grzesiek, mama);
    return 0;
}

TINFRA_MAIN(diff_main);
