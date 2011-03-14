#include <cassert>

template <typename ES>
struct CoolEnum {
	static bool ok(int a) {
		return a >= ES::FIRST && a <= ES::LAST;
	}
};

struct A: CoolEnum<A> {
	enum value_type {
		FIRST = 0,
		FOO   = 0,
		BAR   = 1,
		LAST  = 1
	};
};

int main()
{
	assert( !A::ok(-1) );
	assert(  A::ok(A::FOO) );
	assert(  A::ok(A::BAR) );
	assert( !A::ok(2) );
	assert( !A::ok(3) );
}
