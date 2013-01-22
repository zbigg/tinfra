#include "variant.h" // we implement this

#include <ostream>

namespace tinfra {

//
// variant
//

variant& variant::operator[](std::string const& key) {
    return this->get_dict()[key];
}


variant const& variant::operator[](std::string const& key) const {
    const dict_type& dict = this->get_dict();
    
    dict_type::const_iterator iv = dict.find(key);
    TINFRA_ASSERT(iv != dict.end());
    return iv->second;
}


variant& variant::operator[](int index)
{
    array_type& array = this->get_array();
    if( index == (int)array.size() ) {
        array.push_back(variant());
    } 
    TINFRA_ASSERT(index >= 0 && index < (int)array.size());
    return array[index];
}

variant const& variant::operator[](int index) const
{
    const array_type& array = this->get_array();
    TINFRA_ASSERT(index >= 0 && index < (int)array.size());
    return array[index];
}

size_t variant::size() const
{
    if( this->is_dict() ) {
        return this->get_dict().size();
    } else if ( this->is_array() ) {
        return this->get_array().size();
    } else if ( this->is_string() ) {
        return this->get_string().size();
    } else {
        return 0;
    }
}

std::vector<variant::key_type> variant::dict_keys() const
{
    std::vector<variant::key_type> result;
    const dict_type& dict = this->get_dict();
    for( dict_type::const_iterator i = dict.begin(); i != dict.end(); ++i ) {
        result.push_back(i->first);
    }
    return result;
}

std::ostream& operator <<(std::ostream& s, variant const& node)
{
    if( node.is_string() ) {
        return s << node.get_string();
    } else if( node.is_int() ) {
        return s << node.get_int();
    } else {
        return s << "variant(complex ..., TBD)";
    }
}


} // end namespace tinfra

