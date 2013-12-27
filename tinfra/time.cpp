#include "platform.h"
#include "time.h"
#include "assert.h"
#include "os_common.h"

#include <time.h>
#include <ostream>
#include <iomanip>
#include <cerrno>

namespace tinfra {

std::ostream& operator<<(std::ostream& s, time_stamp v)
{
	const unsigned long long seconds  = v.to_seconds();
	const unsigned milliseconds = v.to_milliseconds() % 1000;

	s << seconds << '.' << std::setw(3) << std::setfill('0') << milliseconds;
	return s;
}
std::ostream& operator<<(std::ostream& s, time_duration v)
{
	const long long seconds  = v.seconds();
	int milliseconds = v.milliseconds() % 1000;
	if( milliseconds < 0 )
		milliseconds = -milliseconds;

	s << seconds << '.' << std::setw(3) << std::setfill('0') << milliseconds;
	return s;

}

//
// tbd, on unix clock_get_time & gettimeofday
//
// on windows, GetSystemTimeAsFileTime &
//   http://support.microsoft.com/kb/167296
//   http://www.lochan.org/2005/keith-cl/useful/win32time.html
//   http://blogs.msdn.com/b/joshpoley/archive/2007/12/19/date-time-formats-and-conversions.aspx

#ifdef _WIN32
time_stamp::value_type ttt_get_system()
{
    time_t x;
    time(&x);
    
    return time_stamp::value_type(x) * time_traits::RESOLUTION;
}
static
time_stamp::value_type ttt_get_monotonic()
{
    return ttt_get_system();
}
#elif defined(HAVE_CLOCK_GETTIME)
time_stamp::value_type ttt_get_system()
{
    struct timespec tv;
    if( clock_gettime(CLOCK_REALTIME, &tv) == -1 ) {
        tinfra::throw_errno_error(errno, "clock_gettime(CLOCK_REALTIME) failed");
    }
    
    // silent assumption that resolution is
    // milliseconds
    TINFRA_STATIC_ASSERT(time_traits::RESOLUTION == 1000);
    
    time_stamp::value_type value 
        =   (time_stamp::value_type(tv.tv_sec) * time_traits::RESOLUTION)
          + (time_stamp::value_type(tv.tv_nsec) / (1000*1000));
    return value;
}
static
time_stamp::value_type ttt_get_monotonic()
{
    struct timespec tv;
    if( clock_gettime(CLOCK_MONOTONIC, &tv) == -1 ) {
        tinfra::throw_errno_error(errno, "clock_gettime(CLOCK_MONOTONIC) failed");
    }
    
    // silent assumption that resolution is
    // milliseconds
    TINFRA_STATIC_ASSERT(time_traits::RESOLUTION == 1000);
    time_stamp::value_type value 
        =   (time_stamp::value_type(tv.tv_sec) * time_traits::RESOLUTION)
          + (time_stamp::value_type(tv.tv_nsec) / (1000*1000));
    return value;
}

#else
static
time_stamp::value_type ttt_get_system()
{
    time_t x;
    time(&x);
    
    return time_stamp::value_type(x) * time_traits::RESOLUTION;
}
static
time_stamp::value_type ttt_get_monotonic()
{
    return ttt_get_system();
}
#endif


time_stamp time_stamp::now(time_source ts)
{
    value_type val = 0;
    switch( ts ) {
    case TS_SYSTEM:    return time_stamp::from_raw(ttt_get_system());
    case TS_MONOTONIC: return time_stamp::from_raw(ttt_get_monotonic());
    default: TINFRA_ASSERT(ts == TS_SYSTEM || ts == TS_MONOTONIC);
    }
}

} // end namespace tinfra
