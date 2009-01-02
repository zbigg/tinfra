//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_generator_h__
#define __tinfra_generator_h__

#include <iterator>
#include <stdexcept>

namespace tinfra {

template <typename UII>
class generator_stl_adapter {
public:
    
    typedef typename UII::value_type value_type;
    typedef value_type&             reference;
    typedef value_type*             pointer;

    typedef std::input_iterator_tag iterator_category;
    typedef void                    difference_type;

    generator_stl_adapter(UII& impl): 
        impl_(&impl),
        current_entry_(0)
    
    {
        fetch_next();
    }
    
    pointer operator->() {
        return current_entry_;
    }
    
    reference operator*() {
        return *current_entry_;
    }
    
    generator_stl_adapter& operator++() {
        fetch_next();
        return *this;
    }
    
    bool operator ==(generator_stl_adapter const& other) {
        return current_entry_ == other.current_entry_;
    }
    
    bool operator !=(generator_stl_adapter const& other) {
        return current_entry_ != other.current_entry_;
    }
    
    static generator_stl_adapter<UII> end() {
        return generator_stl_adapter<UII>();
    }
    
private:
    void fetch_next(){
        if( impl_->has_next() ) {
            current_entry_ = &impl_->next();
        } else {
            current_entry_ = 0;
        }
    }
    
    generator_stl_adapter(): 
        impl_(0),
        current_entry_(0) 
    {
    }
    
    UII*          impl_;
    value_type*   current_entry_;
};

template <typename IMPL, typename T>
class generator_impl {
public:
    typedef T value_type;
    
    generator_impl()
        : last_result_valid_(false) 
    {}
    
    // java-ish iterator contract
    value_type& next() {
        if( !has_next())
            throw std::logic_error("no more elements");
        last_result_valid_ = false;
        return current_entry_;
    }
    
    bool has_next() {
        if( !last_result_valid_) {
            last_result_ = impl().fetch_next(current_entry_);        
            last_result_valid_ = true;        
        }
        return last_result_;    
    }
    
    bool finished() const {
        return last_result_valid_ && !last_result_;
    }
    
    // stl iterator adaptation
    typedef generator_stl_adapter< IMPL > stl_iterator;    
    stl_iterator end() const {
        return stl_iterator::end();
    }
    stl_iterator current() {
        return stl_iterator(impl());
    }

private:
    IMPL& impl() {
        return static_cast<IMPL&>(*this);
    }
    
    value_type current_entry_;
    bool       last_result_valid_;
    bool       last_result_;
};

} // end namespace tinfra

#endif // __tinfra_generator_h__