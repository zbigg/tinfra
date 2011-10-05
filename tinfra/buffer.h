//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_buffer_h_included
#define tinfra_buffer_h_included

#include <algorithm>
#include <iterator>

namespace tinfra {

template <typename T>
class buffer {
    T*          begin_;
    std::size_t size_;
public:
    typedef std::size_t size_type;
    typedef T*          iterator;
    typedef T const*    const_iterator;

    buffer();
    explicit buffer(size_type initial_size);
    buffer(size_type initial_size, T const& pattern);

    buffer(buffer<T> const& other);

    template <typename C>
    explicit buffer(C const&c);

    ~buffer();

    void swap(buffer& other);
    void resize(size_type size, bool keep_old_content = true);

    buffer&       operator=(buffer<T> const& other);

    template <typename C>
    buffer&       operator=(C const& other);

    iterator       data()       { return begin_; }
    const_iterator data() const { return begin_; }

    iterator       begin()       { return begin_; }
    const_iterator begin() const { return begin_; }

    iterator       end()       { return begin_ + size_; }
    const_iterator end() const { return begin_ + size_; }

    size_type      size() const { return size_; }

    T&       operator[](size_type i)       { return *(begin_+i); }
    T const& operator[](size_type i) const { return *(begin_+i); }

    T&       at(size_type i)       { return *(begin_+i); }
    T const& at(size_type i) const { return *(begin_+i); }
};

template <typename T>
bool operator==(buffer<T> const& a, buffer<T> const& b);

//
// buffer implementation
//

template <typename T>
buffer<T>::buffer():
    begin_(0),
    size_(0)
{
}

template <typename T>
buffer<T>::buffer(size_t size):
    begin_(new T[size]),
    size_(size)
{
}

template <typename T>
buffer<T>::buffer(size_type initial_size, T const& pattern):
    begin_(new T[initial_size]),
	size_(initial_size)
{
    for( size_t i = 0; i < initial_size; ++i ) {
	at(i) = pattern;
    }
}

template <typename T>
buffer<T>::buffer(buffer<T> const& other):
    begin_(new T[other.size()]),
    size_(other.size())
{
    std::copy(other.begin(), other.end(), begin());
}

template <typename T>
template <typename C>
buffer<T>::buffer(C const&c):
    begin_(new T[c.size()]),
    size_(c.size())
{
    std::copy(c.begin(), c.end(), begin());
}

template <typename T>
buffer<T>::~buffer()
{
    delete[] begin_;
}

template <typename T>
buffer<T>& buffer<T>::operator=(buffer<T> const& other)
{
    if( this != & other ) {
	resize(other.size(), false);
	std::copy(other.begin(), other.end(), begin());
    }
    return *this;
}
template <typename T>
template <typename C>
buffer<T>& buffer<T>::operator=(C const& other)
{
    resize(other.size(), false);
    std::copy(other.begin(), other.end(), begin());

    return *this;
}

template <typename II, typename OI>
void swap_copy(II const& begin, II const& end, OI const& dest)
{
    II isrc = begin;
    OI idest = dest;

    using std::swap;
    while( isrc != end ) {
	swap(*isrc, *idest);
	++isrc;
	++idest;
    }
}

template <typename T>
void buffer<T>::resize(size_t requested_size, bool keep_old_content)
{
    size_type const current_size = size_;

    if( current_size == requested_size )
	return;

    buffer<T> tmp;
    if( requested_size > 0 ) {
	tmp.resize(requested_size);
	if(  keep_old_content ) {
	    if( requested_size > current_size )
		swap_copy(begin(), begin() + current_size,   tmp.begin());
	    else
		swap_copy(begin(), begin() + requested_size, tmp.begin() );
	}
    }

    swap(tmp);
    // tmp destroys original buffer
}

template <typename T>
void buffer<T>::swap(buffer<T>& other)
{
    using std::swap;
    swap(begin_, other.begin_);
    swap(size_, other.size_);
}

template <typename T>
bool operator==(buffer<T> const& a, buffer<T> const& b)
{
    if( a.size() != b.size() ) 
	return false;

    typename buffer<T>::const_iterator ia = a.begin();
    typename buffer<T>::const_iterator ib = b.begin();

    while( ia != a.end() ) {
	if( ! (*ia == *ib ) ) {
	    return false;
	}
	++ia;
	++ib;
    }
    return true;
}

typedef buffer<char> char_buffer;
typedef buffer<wchar_t> wchar_buffer;

} // end namespace tinfra

// specialize std::swap for this type
namespace std {
    template <typename T>
    void swap(tinfra::buffer<T>& a, tinfra::buffer<T>& b)
    {
	a.swap(b);
    }
}

#endif // tinfra_buffer_h_included

