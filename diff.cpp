#include <tinfra/symbol.h>
#include <tinfra/cmd.h>

#include <iostream>
#include <tinfra/mo.h>

TINFRA_SYMBOL_DECL(street);
TINFRA_SYMBOL_DECL(home);
TINFRA_SYMBOL_DECL(apartment);
TINFRA_SYMBOL_DECL(city);

struct address {
    std::string street;
    std::string home;
    std::string apartment;
    std::string city;
    
    TINFRA_MO_MANIFEST(address)
    {
        TINFRA_MO_FIELD(street);
        TINFRA_MO_FIELD(home);
        TINFRA_MO_FIELD(apartment);
        TINFRA_MO_FIELD(city);
    }
};


TINFRA_SYMBOL_IMPL(street);
TINFRA_SYMBOL_IMPL(home);
TINFRA_SYMBOL_IMPL(apartment);
TINFRA_SYMBOL_IMPL(city);

namespace tinfra {
    template<> 
    struct mo_traits<address>: public tinfra::struct_mo_traits<address> {};
} 

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
    template <typename T>
    void mstruct(symbol const&, T const& value) {
        tinfra::mo_process(value, *this);
    }
};


template <typename T>
void compare(tinfra::symbol const& sym, T const& a, T const& b)
{
    comparator c(&a,&b);
    tinfra::process(sym, a,c);
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
        using std::swap;
        swap(va, vb);
    }
};

template <typename T>
void tinfra_swap(T& a, T& b)
{
    swapper s(&a, &b);
    tinfra::mo_mutate(a, s);
}


int diff_main(int, char**)
{
    using tinfra::symbol;
    address grzesiek = { "garlicka", "12b", "", "Tanowo" };
    address mama = { "malpia", "12", "", "Szczein" };
    
    compare(symbol("abc"), grzesiek, mama);
    tinfra_swap(grzesiek, mama);
    compare(symbol("abc"), grzesiek, mama);
    return 0;
}

TINFRA_MAIN(diff_main);

