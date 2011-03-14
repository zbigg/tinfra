#include <iostream>

#if 1
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif
template <typename R>
struct F0 {
	R operator()() {
		std::cout << "calling: " <<  __PRETTY_FUNCTION__ << "\n";
		R r;
		return r;
	}
};

template <typename R, typename T1>
struct F1 {
	R operator()(T1 const&) {
		R r;
		std::cout << "calling: " <<  __PRETTY_FUNCTION__ << "\n"; 
		return r;
	}
};

template <typename R, typename T1, typename T2>
struct F2 {
	R operator()(T1, T2) {
		std::cout << "calling: " <<  __PRETTY_FUNCTION__ << "\n";
		R r;
		return r;
	}
};

struct FOO {};

template <typename RET, typename T1 = FOO, typename T2 = FOO>
class F;

template <typename RET>
class F<RET()> : public F0<RET>
{	
};

template <typename RET, typename T1>
class F<RET (T1)> : public F1<RET, T1>
{	
};

template <typename RET, typename T1, typename T2>
class F<RET (T1, T2)> : public F2<RET, T1, T2>
{	
};

int main()
{
	F<int(const FOO&,const char*)> X;
	FOO a;
	X(a,"abc");
	return 0;
}

