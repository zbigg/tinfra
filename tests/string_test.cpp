//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/string.h" // API under test
#include "tinfra/fmt.h"

#include "tinfra/test.h" // test infra

using tinfra::fmt;

SUITE(tinfra)
{
    std::string strip(std::string const& s)
    {
        std::string r1 = tinfra::strip(s);
        std::string r2(s);
        tinfra::strip_inplace(r2);
        CHECK_EQUAL( r1, r2 );
        return r1;
    }
    
    TEST(string_strip)
    {
        CHECK_EQUAL("", strip("     "));
        CHECK_EQUAL("abc", strip("abc"));
        CHECK_EQUAL("abc", strip(" abc"));
        CHECK_EQUAL("abc", strip("abc "));
        CHECK_EQUAL("a b c", strip("a b c"));
        CHECK_EQUAL("a b c", strip("  a b c  "));
    }
    
    std::string escape_c(std::string const& s)
    {
        std::string r1 = tinfra::escape_c(s);
        std::string r2(s);
        tinfra::escape_c_inplace(r2);
        CHECK_EQUAL( r1, r2 );
        return r1;
    }
    
    TEST(string_escape_c)
    {
        CHECK_EQUAL("abc", escape_c("abc"));
        CHECK_EQUAL("a\\nc", escape_c("a\nc"));
        CHECK_EQUAL("abc\\r\\n", escape_c("abc\r\n"));
        CHECK_EQUAL("a\\x1c", escape_c("a\x01c"));
        CHECK_EQUAL("abc\\xff", escape_c("abc\xff"));        
    }
    
    std::string chop(std::string const& s)
    {
        std::string r1 = tinfra::chop(s);
        std::string r2(s);
        tinfra::chop_inplace(r2);
        CHECK_EQUAL( r1, r2 );
        return r1;
    }
    
    TEST(string_chop)
    {
        CHECK_EQUAL(" ", chop(" "));
        CHECK_EQUAL("", chop(""));
        CHECK_EQUAL("abc", chop("abc"));
        CHECK_EQUAL("abc", chop("abc\n"));
        CHECK_EQUAL("abc", chop("abc\r"));
        CHECK_EQUAL("abc", chop("abc\r\n"));
        CHECK_EQUAL("abc", chop("abc\n\r"));
        CHECK_EQUAL("abc", chop("abc\n\r"));
        CHECK_EQUAL("", chop("\r"));
        CHECK_EQUAL("", chop("\r\n"));
    }
    
    TEST(string_split_lines)
    {
        using tinfra::split_lines;
        using std::vector;
        using std::string;
        
        vector<string> r;
        r = split_lines("");
        CHECK_EQUAL(1u, r.size() );
        
        r = split_lines("a");
        CHECK_EQUAL(1u, r.size() );
        CHECK_EQUAL("a", r[0] );
        
        r = split_lines("a\r"); // not a full end of line so we even don't detect it!
        CHECK_EQUAL(1u, r.size() );
        CHECK_EQUAL("a\r", r[0] );
        
        r = split_lines("a\n");
        CHECK_EQUAL(1u, r.size() );
        CHECK_EQUAL("a", r[0] );
        
        r = split_lines("a\r\n");
        CHECK_EQUAL(1u, r.size() );
        CHECK_EQUAL("a", r[0] );
        
        r = split_lines("a\r\nb");
        CHECK_EQUAL(2u, r.size() );
        CHECK_EQUAL("a", r[0] );
        CHECK_EQUAL("b", r[1] );
    }

    TEST(string_split_lines_two_delims)
    {
        using tinfra::split_lines;
        using std::vector;
        using std::string;
        
        vector<string> r;
        r = split_lines("");
        CHECK_EQUAL(1u, r.size() );
        
        r = split_lines("FIRST\n\nTHIRD\n");
        CHECK_EQUAL(3u, r.size() );
        CHECK_EQUAL("FIRST", r[0] );
        CHECK_EQUAL("",  r[1] );
        CHECK_EQUAL("THIRD", r[2] );
    }    
    TEST(string_split_strict_two_delims)
    {
        using tinfra::split_strict;
        using std::vector;
        using std::string;
        
        vector<string> r;
        r = split_strict("",'|');
        CHECK_EQUAL(1u, r.size() );
        
        r = split_strict("FIRST||THIRD|",'|');
        CHECK_EQUAL(4u, r.size() );
        CHECK_EQUAL("FIRST", r[0] );
        CHECK_EQUAL("",  r[1] );
        CHECK_EQUAL("THIRD", r[2] );
        CHECK_EQUAL("",  r[1] );
    }

    TEST(string_split_skip_empty_two_delims)
    {
        using tinfra::split_skip_empty;
        using std::vector;
        using std::string;
        
        vector<string> r;
        r = split_skip_empty("",',');
        CHECK_EQUAL(1u, r.size() );
        
        r = split_skip_empty("FIRST,,THIRD,",',');
        CHECK_EQUAL(3u, r.size() );
        CHECK_EQUAL("FIRST", r[0] );
        CHECK_EQUAL("THIRD", r[1] );
        CHECK_EQUAL("", r[2] );
    }
    
    TEST(string_before_first)
    {
        using tinfra::before_first;
        
        // main use case
        CHECK_EQUAL("mount", before_first(" \t", "mount /dev/cdrom"));
        // and variations
        CHECK_EQUAL("mount", before_first(" \t", "mount\t/dev/cdrom"));
        
        // and edge cases
        CHECK_EQUAL("mountcdrom", before_first(" \t", "mountcdrom"));
        CHECK_EQUAL("", before_first("m", "mountcdrom"));
    }
    
    TEST(string_compare_no_case_corner)
    {
        using tinfra::compare_no_case;
        CHECK_EQUAL( 0, compare_no_case("","") );
        CHECK_EQUAL( 0, compare_no_case("A","B", 0) );
        CHECK_EQUAL( 0, compare_no_case("A","", 0) );
        CHECK_EQUAL( 0, compare_no_case("","B", 0) );
    }
    
    TEST(string_compare_no_case_basic)
    {
        using tinfra::compare_no_case;
        CHECK_EQUAL( 0, compare_no_case("abc","ABC") );
        CHECK_EQUAL( 0, compare_no_case("abc ZXY","ABC zxy") );
        CHECK_EQUAL( 0, compare_no_case("defGEG","defGEG___", 6) );
        
        CHECK( compare_no_case("defGEG","defGEGaa") < 0);
        CHECK( compare_no_case("defGEGaa","defGEG") > 0);

        CHECK( compare_no_case("defGEG","defGEGaa") < 0);
        CHECK( compare_no_case("defGEGaa","defGEG") > 0);
    }
}
