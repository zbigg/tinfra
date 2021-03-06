//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include <iostream>
#include <stdexcept>

#include "tinfra/fmt.h"

#include "tinfra/regexp.h"

namespace tinfra {
//
// matcher
//

matcher::matcher(regexp const& re, const char* str, size_t length):
    re_(re),
    str_(str),
    length_(length),
    position_(0),
    have_result_(false),
    have_match_(false)
{}

matcher::matcher(regexp const& re, const char* str):
    re_(re),
    str_(str),
    length_(std::strlen(str)),
    position_(0),
    have_result_(false),
    have_match_(false)
{}

std_match_result const& matcher::next() {
    if( ! have_result_ ) 
        try_match();
    if( have_match_ ) {
        have_result_ = false;
        return match_;
    } else {
        throw std::runtime_error("no more matches");
    }
}

bool matcher::has_next()
{
    if( !have_result_ ) 
        try_match();
    return have_match_;
}


void matcher::try_match()
{
    have_result_ = true;
    if( position_ == length_ ) {
        have_match_ = false;
    } else {
        size_t match_end;
        have_match_ = re_.do_match(&match_, 
                                   str_ + position_, length_ - position_,
                                   &match_end);
        position_ = position_ + match_end;
    }
}

//
// scanner
//



void scanner::main_groups_collector::prepare(int)
{
	// actually we don't believe in what PCRE is saying because it
	// tells us all group count
}
//
// this collect only "non-zero" groups, example analysis
// AAAAAA       group 0
//  BBBBB       ignored 
//    CC        ignored
//       DDDDD  group 1
//          FF  ignored
//  sumamry 2 groups

void scanner::main_groups_collector::match_group(int group_no, const char* str, size_t pos, size_t len)
{
	if( group_no == 0 ) { // group 0 is not interesting ! 
		return;
	}
	
	if( group_no == 1 ) { // this one is always added
		const char* begin = str+pos;
		this->main_groups.push_back(tstring(begin, len));
		return;
	}  
	const tstring& last_group = this->main_groups[this->main_groups.size()-1];
	const char* last_group_end = last_group.data() + last_group.size();
	const char* current_begin = str+pos;
	if( current_begin < last_group_end ) {
		return;
	}
	this->main_groups.push_back(tstring(current_begin, len));
}

bool scanner::check_and_forward()
{
    if( !this->have_match_ ) 
        return false;
    
    if( this->current_param_ == this->match_.main_groups.size() ) {
        throw std::logic_error("scanner: too many arguments");
    }
    
    return true;
}

//
// std_match_result
//

void std_match_result::prepare(int g)
{
    groups.resize(g);
}

void std_match_result::match_group(int group_no, const char* str, size_t pos, size_t len)
{
    const char* begin = str + pos;
    groups[group_no].assign(begin, len);
}

} // end namespace tinfra

