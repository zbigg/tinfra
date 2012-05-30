strangly typed C++
==================

The general idea is that types should be strong i.e
 - first_name 
 - last_name
 - street_name
Are different types although represented using string.

But OTOH, these can have different physical representation:
 - const char*
 - tstring
 - string
 - basic_string<custom_allocator>
 - wxString
 - QString ...
 
This type somehow affects real values that can be carried.

So a method, paradigm, framework for shall be created
for types that are semantically same (think:
  first_name
  internet_domain_name<std::string>
  internet_domain_name<wxString>
)
but with different representation.

----
some notes from thinking about SI units, meter:

si::meter<double>
    
si::meter<int>
    useable for arithmetic

si::meter<string> 
    usable in ???
    use case is standalone version that can be edit
        
    
    
template <typename Storage>
class meter {
    template <typename SourceStorage>
    from
    template <typename TargetStorage>
    get(
    S storage;
};

template <typename A, typename B>
 
