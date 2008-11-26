//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/string.h"
#include "tinfra/tstring.h"

#include <cstdio>
#include <cstring>
#include <cctype>
#include <iostream>

namespace tinfra {
 
//
// strip implementation
//    
    
static const char* whitespace = " \t\r\n\v";

void strip_inplace(std::string& s)
{    
    {
        std::size_t pos = s.find_first_not_of(whitespace);
        if( pos != 0 )
            s.erase(0,pos);
    }
    {
        std::size_t pos = s.find_last_not_of(whitespace);
        if( pos != std::string::npos ) 
            s.erase(pos+1);
    }
}

std::string strip(tstring const& s)
{
    
    std::size_t start = s.find_first_not_of(whitespace);
    if( start == std::string::npos )
        return std::string("");
    std::size_t last = s.find_last_not_of(whitespace);
    if( last != std::string::npos )
        last = last-start+1;
        
    return std::string(s.begin(), start, last);
}

static const char* END_OF_LINE_CHARS = "\r\n";
//
// chop implementation
//

void        chop_inplace(std::string& s)
{
    std::size_t pos = s.find_last_not_of(END_OF_LINE_CHARS);
    if( pos == std::string::npos ) {
        s.clear();
    } else {
        s.erase(pos+1);
    }
}

std::string chop(tstring const& s)
{
    std::size_t start = 0;
    std::size_t last = s.find_last_not_of(END_OF_LINE_CHARS);
    if( last == std::string::npos ) {
        /* if(     s.size() > 0 
            && (s[0] == '\r' || s[0] == '\n' ) )
            return std::string();
        return s; */
        return std::string();
    }
    ++last;
    return std::string(s.begin(), 0, last);
}

// escape_c
//

void escape_c_inplace(std::string& a)
{
    std::size_t i = 0;
    while( i < a.size() ) {
        if( a[i] == '\n' ) {
            a.replace(i, 1, "\\n");
            i +=2;
        } else if( a[i] == '\r' ) {
            a.replace(i, 1, "\\r");
            i +=2;
        } else if( !std::isprint(a[i]) ) {
            char buf[20];
            std::sprintf(buf, "\\x%02x", (int)(unsigned char)a[i]);
            a.replace(i, 1, buf);
            i += std::strlen(buf);
        } else {
            i +=1;
        }
    }
}

std::string escape_c(tstring const& a)
{
    std::string result(a.begin(), a.size());
    escape_c_inplace(result);
    return result;
}

std::vector<std::string> split(tstring const& in, tstring const& delimiters)
{
    std::vector<std::string> result;
    std::size_t start = 0;
    do {
        std::size_t pos = in.find_first_of(delimiters, start);
        result.push_back(std::string(in.begin(), start, pos-start));
        
        start = pos;
        
        if( start != std::string::npos ) {
            start = in.find_first_not_of(delimiters, start);
        }
    } while( start != std::string::npos );
        
    return result;
}

std::vector<std::string> split_lines(tstring const& in)
{
    return split(in, "\r\n");
}

} // end namespace tinfra
