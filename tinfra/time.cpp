#include "platform.h"
#include "time.h"

#include <ostream>
#include <iomanip>

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

} // end namespace tinfra
