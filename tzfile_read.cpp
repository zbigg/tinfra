#include <vector>
#include <tinfra/stream.h>
#include <tinfra/mo.h>
#include <tinfra/structure_printer.h>

#include <sstream>

#include "stdint.h"

using std::vector;

struct tz_header {
    char     tzh_magic[4];
    char     tzh_version;
    unsigned char tzh_future[15];
    
    long tzh_ttisgmtcnt;
    long tzh_ttisstdcnt;
    long tzh_leapcnt;
    long tzh_timecnt;
    long tzh_typecnt;
    long tzh_charcnt;
    
    TINFRA_MO_MANIFEST(tz_header) {
        TINFRA_MO_FIELD(tzh_magic);
        TINFRA_MO_FIELD(tzh_version);
        TINFRA_MO_FIELD(tzh_future);
        
        TINFRA_MO_FIELD(tzh_ttisgmtcnt);
        TINFRA_MO_FIELD(tzh_ttisstdcnt);
        TINFRA_MO_FIELD(tzh_leapcnt);
        TINFRA_MO_FIELD(tzh_timecnt);
        TINFRA_MO_FIELD(tzh_typecnt);
        TINFRA_MO_FIELD(tzh_charcnt);
    }
};

TINFRA_MO_IS_RECORD(tz_header);

struct tz_ttinfo {
    long         tt_gmtoff;
    int          tt_isdst;
    unsigned int tt_abbrind;
    
    TINFRA_MO_MANIFEST(tz_ttinfo) {
        TINFRA_MO_FIELD(tt_gmtoff);
        TINFRA_MO_FIELD(tt_isdst);
        TINFRA_MO_FIELD(tt_abbrind);        
    }
};

TINFRA_MO_IS_RECORD(tz_ttinfo);

struct tz_leapsecond_info {
    long tlsi_time;
    long tlsi_leap_seconds;
    
    TINFRA_MO_MANIFEST(tz_leapsecond_info) {
        TINFRA_MO_FIELD(tlsi_time);
        TINFRA_MO_FIELD(tlsi_leap_seconds);   
    }
};

TINFRA_MO_IS_RECORD(tz_leapsecond_info);

struct tz_file {
    tz_header         header;
    vector<long>   transition_times;
    vector<unsigned char>   transition_ttinfo_indices;
    vector<tz_ttinfo> ttinfo;
    vector<char>      char_abbreviations;
    vector<tz_leapsecond_info> leap_seconds;
    vector<unsigned char>   standard_wall_indicators;
    vector<unsigned char>   utc_local_indicators;
    
    TINFRA_MO_MANIFEST(tz_file) {
        TINFRA_MO_FIELD(header);
        TINFRA_MO_FIELD(transition_times);
        TINFRA_MO_FIELD(transition_ttinfo_indices);
        TINFRA_MO_FIELD(ttinfo);
        TINFRA_MO_FIELD(char_abbreviations);
        TINFRA_MO_FIELD(leap_seconds);
        TINFRA_MO_FIELD(standard_wall_indicators);
        TINFRA_MO_FIELD(utc_local_indicators);
    }
};

TINFRA_MO_IS_RECORD(tz_file);

template <typename T>
void read(tinfra::input_stream& s, T& v)
{
    s.read((char*)&v, sizeof(v));
}

void read_long(tinfra::input_stream& s, long& v)
{
    uint8_t bytes[4] = {0};
    read(s, bytes);
    
    v = ((int32_t)bytes[0] << 24) |
        ((int32_t)bytes[1] << 16) |
        ((int32_t)bytes[2] << 8)  |
        ((int32_t)bytes[3]);
}
void read_header(tinfra::input_stream& s, tz_header& dst)
{
    read(s, dst.tzh_magic);
    read(s, dst.tzh_version);
    read(s, dst.tzh_future);
    read_long(s, dst.tzh_ttisgmtcnt);
    read_long(s, dst.tzh_ttisstdcnt);
    read_long(s, dst.tzh_leapcnt);
    read_long(s, dst.tzh_timecnt);
    read_long(s, dst.tzh_typecnt);
    read_long(s, dst.tzh_charcnt);
}

void read_ttinfo(tinfra::input_stream& s, tz_ttinfo& dst)
{
    char isdst, abbrind;
    read_long(s, dst.tt_gmtoff);    
    read(s, isdst);
    read(s, abbrind);
    dst.tt_isdst = isdst;    
    dst.tt_abbrind = abbrind;
}

void read_tzfile(tinfra::input_stream& s, tz_file& dst)
{
    read_header(s, dst.header);
    
    dst.transition_times.resize(dst.header.tzh_timecnt);
    dst.transition_ttinfo_indices.resize(dst.header.tzh_timecnt);
    dst.ttinfo.resize(dst.header.tzh_typecnt);
    
    // transition times
    for( int i = 0 ; i < dst.header.tzh_timecnt; ++i ) {
        read_long(s, dst.transition_times[i]);
    }
    
    // transition ttinfo indices
    for( int i = 0 ; i < dst.header.tzh_timecnt; ++i ) {
        read(s, dst.transition_ttinfo_indices[i]);
    }
    
    // transition ttinfo indices
    for( int i = 0 ; i < dst.header.tzh_typecnt; ++i ) {
        read_ttinfo(s, dst.ttinfo[i]);
    }
    dst.char_abbreviations.resize(dst.header.tzh_charcnt);
    for( int i = 0 ; i < dst.header.tzh_charcnt; ++i ) {
        read(s, dst.char_abbreviations[i]);
    }
    
    //
    dst.leap_seconds.resize(dst.header.tzh_leapcnt);
    for( int i = 0 ; i < dst.header.tzh_leapcnt; ++i ) {
        read_long(s, dst.leap_seconds[i].tlsi_time);
        read_long(s, dst.leap_seconds[i].tlsi_leap_seconds);
    }
    
    dst.standard_wall_indicators.resize(dst.header.tzh_ttisstdcnt);
    for( int i = 0 ; i < dst.header.tzh_ttisstdcnt; ++i ) {
        read(s, dst.standard_wall_indicators[i]);
    }
    
    dst.utc_local_indicators.resize(dst.header.tzh_ttisgmtcnt);
    for( int i = 0 ; i < dst.header.tzh_ttisgmtcnt; ++i ) {
        read(s, dst.utc_local_indicators[i]);
    }
}

struct numeric_formatter {
    template <typename T>
    void operator()(std::ostream& str, T const& value)
    {
        str << value;
    }
    
    void operator()(std::ostream& str, unsigned char const& value)
    {
        str << (unsigned int)value;
    }
    void operator()(std::ostream& str, char const& value)
    {
        str << (char)value;
    }
};
int main(int argc, char** argv)
{
    using std::auto_ptr;
    using tinfra::input_stream;
    auto_ptr<input_stream> file(tinfra::create_file_input_stream(argv[1]));
    
    tz_file tzf;
    read_tzfile(*file, tzf);
    
    std::ostringstream buf;
    tinfra::basic_structure_printer<numeric_formatter> printer(buf);
    printer.set_multiline(true);
    
    tinfra::process("tz", tzf, printer);
    
    tinfra::out.write(buf.str());
}
