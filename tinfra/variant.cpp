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

bool operator==(variant const& a, variant const& b)
{
    if( a.is_none() && b.is_none() )
        return true;
    else if ( a.is_string() ) {
        return b.is_string() && a.get_string() == b.get_string();
    } else if( a.is_integer() ) {
        return b.is_integer() && a.get_integer() == b.get_integer();
    } else if( a.is_double() ) {
        return b.is_double() && a.get_double() == b.get_double();
    } else if( a.is_dict() ) {
        if( b.is_dict() ) {
            variant::dict_type const& ad = a.get_dict();
            variant::dict_type const& bd = b.get_dict();
            if( ad.size() != bd.size() )
                return false;
            return std::equal(ad.begin(), ad.end(), bd.begin());
        } else {
            return false;
        }
    } else if( a.is_array())  {
        if( b.is_array() ) {
            variant::array_type const& aa = a.get_array();
            variant::array_type const& ba = b.get_array();
            if( aa.size() != ba.size() )
                return false;
            return std::equal(aa.begin(), aa.end(), ba.begin());
        } else {
            return false;
        }
    } else {
        return false;
    }
}
bool operator!=(variant const& a, variant const& b)
{
    return !(a == b);
}
std::ostream& operator <<(std::ostream& s, variant const& node)
{
    if( node.is_string() ) {
        return s << node.get_string();
    } else if( node.is_integer() ) {
        return s << node.get_integer();
    } else if( node.is_double() ) {
        return s << node.get_double();
    } else if( node.is_dict() ) {
        return s << "<dictionary>";
    } else if( node.is_array() ) {
        return s << "<array>";
    } else {
        return s << "variant(?)";
    }
}


} // end namespace tinfra

