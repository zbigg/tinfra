#include "tinfra/time.h" // we test this

#include "tinfra/test.h" // test infra

SUITE(tinfra) {

TEST(time_duration_basics)
{
	using tinfra::time_duration;
	time_duration s = time_duration::second(1);
	time_duration h = time_duration::hour(1);

	CHECK_EQUAL(h, s*60*60);
}
/*
TEST(time_duration_ops)
{
    using tinfra::time_stamp;
    using tinfra::time_duration;

    time_stamp t0 = time_duration::from_seconds_since_epoch(101);
    time_stamp t1 = time_duration::from_seconds_since_epoch(200);
    
    time_duration dt = t1-t0;        
    CHECK_EQUAL(time_duration::second(99), dt);

    CHECK_EQUAL(t1, t0 + dt);
    CHECK_EQUAL(t0, t1 - dt);
}
*/
TEST(time_stamp_relations)
{
	using tinfra::time_stamp;
	using tinfra::time_duration;

	time_stamp t0 = time_stamp::from_seconds_since_epoch(101);
	time_stamp t1 = time_stamp::from_seconds_since_epoch(200);

	CHECK_EQUAL(time_duration::second(99), t1-t0);
}


TEST(time_stamp_operators)
{
	using tinfra::time_stamp;
	using tinfra::time_duration;

	time_stamp t0 = time_stamp::from_seconds_since_epoch(101);
	time_stamp t1 = time_stamp::from_seconds_since_epoch(200);

	CHECK_EQUAL(time_duration::second(99), t1-t0);
}

} // 

