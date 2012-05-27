//
// Copyright (c) 2012, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_time_h_included
#define tinfra_time_h_included

#include <iosfwd>

namespace tinfra {

typedef unsigned long long time_uint;
typedef signed   long long time_int;

struct time_traits {
	typedef time_uint value_type;
	typedef time_int  difference_type;

	enum definitions {
		RESOLUTION = 1000
	};
};

/// a point in POSIX time
class time_stamp;

/// a duration between two POSIX time points
class time_duration;

class time_stamp {
public:
	typedef time_traits::value_type value_type;

	// constructors
	time_stamp();
	static time_stamp from_seconds_since_epoch(time_uint);
	static time_stamp from_raw(time_uint);

	time_uint to_seconds() const;
	time_uint to_milliseconds() const;
	time_uint to_raw() const;
private:
	// unitless constructor only for 
	// static constuctors
	time_stamp(value_type v);

	value_type t;
};

class time_duration {
public:
	typedef time_traits::difference_type value_type;

	/// constructs a 0-length duration
	time_duration();

	static time_duration day(value_type days);
	static time_duration hour(value_type hours);
	static time_duration minute(value_type hours);
	static time_duration second(value_type seconds);
	static time_duration millisecond(value_type v);
	static time_duration microsecond(value_type v);
	static time_duration nanosecond(value_type v);
	static time_duration from_raw(value_type v);


	value_type days() const;
	value_type hours() const;
	value_type seconds() const;
	value_type milliseconds() const;
	value_type microseconds() const;
	value_type nanoseconds() const;
	value_type to_raw() const;
private:
	// unitless constructor only for 
	// static constuctors
	time_duration(value_type v);

	value_type dt;
};

// time_duration operations
bool operator==(time_duration, time_duration);
bool operator!=(time_duration, time_duration);
bool operator<(time_duration, time_duration);
bool operator>(time_duration, time_duration);
bool operator>=(time_duration, time_duration);
bool operator<=(time_duration, time_duration);

time_duration operator+(time_duration, time_duration);
time_duration operator-(time_duration, time_duration);
time_duration operator*(time_duration, int);
time_duration operator*(int, time_duration);
time_duration operator/(time_duration, int);
time_duration operator/(int, time_duration);

time_duration& operator+=(time_duration&, time_duration);
time_duration& operator-=(time_duration&, time_duration);

// time_stamp operations
bool operator==(time_stamp, time_stamp);
bool operator!=(time_stamp, time_stamp);
bool operator<(time_stamp, time_stamp);
bool operator>(time_stamp, time_stamp);
bool operator>=(time_stamp, time_stamp);
bool operator<=(time_stamp, time_stamp);

/// move point forward/backward in time
time_stamp& operator+=(time_stamp&, time_duration);
time_stamp& operator-=(time_stamp&, time_duration);

// time_stamp+time_duration operations
time_stamp   operator+(time_duration, time_stamp);
time_stamp   operator+(time_stamp, time_duration);
time_stamp   operator-(time_duration, time_stamp);
time_stamp   operator-(time_stamp, time_duration);

time_duration operator-(time_stamp, time_stamp);

// std::iostream output adaptors
std::ostream& operator<<(std::ostream&, time_stamp);
std::ostream& operator<<(std::ostream&, time_duration);

//
// time_stamp (inline implementation)
//

inline
time_stamp::time_stamp(): t(0) {}

inline
time_stamp::time_stamp(time_stamp::value_type v): t(v) {}

inline
time_stamp time_stamp::from_seconds_since_epoch(time_uint tv)
{
	return time_stamp(tv*time_traits::RESOLUTION);
}

inline
time_stamp time_stamp::from_raw(time_uint tv) {
	return time_stamp(tv);
}


inline time_uint 
time_stamp::to_seconds()      const { return this->t/time_traits::RESOLUTION; }

inline time_uint
time_stamp::to_milliseconds() const { return (this->t*1000)/time_traits::RESOLUTION; }

inline time_uint
time_stamp::to_raw() const { return this->t; }

inline time_duration
operator - (time_stamp a, time_stamp b)
{
	return time_duration::from_raw(
		a > b ?
			a.to_raw() - b.to_raw()
		      : 
		        - static_cast<time_uint>(b.to_raw() - a.to_raw())
		);
}

inline bool operator==(time_stamp a, time_stamp b) { return a.to_raw() == b.to_raw(); }
inline bool operator!=(time_stamp a, time_stamp b) { return a.to_raw() != b.to_raw(); }
inline bool operator< (time_stamp a, time_stamp b) { return a.to_raw() <  b.to_raw(); }
inline bool operator> (time_stamp a, time_stamp b) { return a.to_raw() >  b.to_raw(); }
inline bool operator>=(time_stamp a, time_stamp b) { return a.to_raw() >= b.to_raw(); }
inline bool operator<=(time_stamp a, time_stamp b) { return a.to_raw() <= b.to_raw(); }

//
// time_duration (inline implementation)
//

inline
time_duration::time_duration(): dt(0) {}

inline
time_duration::time_duration(time_duration::value_type v): dt(v) {}

inline time_duration
time_duration::second(time_int tv) { return time_duration(tv*time_traits::RESOLUTION); }

inline time_duration
time_duration::minute(time_int tv) { return time_duration::second(60*tv); }

inline time_duration
time_duration::hour(time_int tv) { return time_duration::minute(60*tv); }

inline time_duration
time_duration::from_raw(time_int tv) { return time_duration(tv); }

inline bool operator==(time_duration a, time_duration b) { return a.to_raw() == b.to_raw(); }
inline bool operator!=(time_duration a, time_duration b) { return a.to_raw() != b.to_raw(); }
inline bool operator< (time_duration a, time_duration b) { return a.to_raw() <  b.to_raw(); }
inline bool operator> (time_duration a, time_duration b) { return a.to_raw() >  b.to_raw(); }
inline bool operator>=(time_duration a, time_duration b) { return a.to_raw() >= b.to_raw(); }
inline bool operator<=(time_duration a, time_duration b) { return a.to_raw() <= b.to_raw(); }


// TBD time_duration DAY, micro, nanosecond

inline time_int
time_duration::seconds() const { return this->dt/time_traits::RESOLUTION; }

inline time_int
time_duration::milliseconds() const { return (this->dt*1000)/time_traits::RESOLUTION; }

inline time_int
time_duration::to_raw() const { return this->dt; }


//
// time_stamp vs time_duration (inline implementation)
//

inline 
time_duration operator*(time_duration a, int b) { return time_duration::from_raw(a.to_raw() * b); }

inline 
time_duration operator/(time_duration a, int b) { return time_duration::from_raw(a.to_raw() / b); }


	
} // end namespace tinfra

#endif
