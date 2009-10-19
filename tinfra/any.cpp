//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/any.h"

#include <cassert>

namespace tinfra {

//
// any_container_base
// 
any_container_base::~any_container_base()
{
}

//
// any
//

any::any(any_container_base* ptr)
        : ref_(ptr)
{ }

void* any::get_raw() {
    assert(ref_.get() != 0);
    return ref_->get();
}

const void* any::get_raw() const {
    assert(ref_.get() != 0);
    return ref_->get();
}

std::type_info const& any::type() const {
    assert(ref_.get() != 0);
    return ref_->type();
}    
    
} // end namespace tinfra
