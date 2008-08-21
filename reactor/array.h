#ifndef __tinfra__array_h_
#define __tinfra__array_h_

template <typename T>
class array {    
public:
    typedef T value_type;
    
    typedef value_type* iterator;
    typedef value_type const* const_iterator;
    typedef std::size_t size_type;

    iterator begin() { return _begin; }
    const_iterator begin() const { return _begin; }
    
    iterator end() { return _end; }
    const_iterator end() const { return _end; }
    
    size_type size() const { return _end - _begin; }
    
    void size(size_t new_size) {
        if( size() != new_size ) {
            iterator new_begin = new T[new_size];
            
            if( size() > 0 ) {
                size_type common_size = size() < new_size ? size() : new_size; // TODO: replace with std::min when i'll find it
                std::copy(_begin, _begin + common_size, new_begin);
                delete [] _begin;
            }
            _begin = new_begin;
            _end = _begin + new_size;
        }
    }
    
    array(size_type n = 0): _begin(0), _end(0) { size(n); }
    array(array<T> const& other): _begin(0), _end(0) { size(other.size()); std::copy(other.begin(), other.end(), begin()); }
    ~array() { delete [] _begin; }
    
    T&       at(size_type n)       { return *(_begin + n); }
    T const& at(size_type n) const { return *(_begin + n); }
    
    T&       operator[] (size_type n)       { return at(n); }
    T const& operator[] (size_type n) const { return at(n); }
    
private:
    iterator _begin;
    iterator _end;
};

#endif
