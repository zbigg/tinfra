//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/platform.h"
#include "tinfra/string.h"
#include "tinfra/tstring.h"

#include <cstdio>
#include <cstring>
#include <cctype>

#if defined(HAVE_STRICMP) || defined(_MSC_VER) || defined(HAVE_STRCASECMP)
#include <string.h>
#endif


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

static int local_strcasecmp(const char* a, const char* b, size_t len)
{
#if defined(HAVE_STRICMP) || defined(_MSC_VER)
    return strnicmp(a, b, len);
#elif defined(HAVE_STRCASECMP)
    return strncasecmp(a, b, len);
#else
    size_t idx = 0;
    while( idx != len ) {
        const char ca = std::tolower(*a);
        const char cb = std::tolower(*b);
        if( ca != cb )
            return ca - cb;
        ++b, ++a, ++idx;
    }
    return 0;
#endif
}

int         compare_no_case(tstring const& a, tstring const& b)
{
    const size_t common_length = std::min(a.size(), b.size());
    const int result = local_strcasecmp(a.data(), b.data(), common_length);
    if( result != 0 )
        return result;
    else if( a.size() == b.size() )
        return 0;
    else
        return a.size() > b.size() ? 1 : -1;
}

int         compare_no_case(tstring const& a, tstring const& b, size_t upto)
{
    return local_strcasecmp(a.data(), b.data(), upto);
}

} // end namespace tinfra
