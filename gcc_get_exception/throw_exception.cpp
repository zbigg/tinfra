#include <vector>
#include <string>
#include <unistd.h>

extern int back_call(int);

void throw_an_exception(int now_or_later)
{
	std::vector<std::string> xxx(100);
	int z = now_or_later;
	
	::close(now_or_later+100+z-z);
	if( now_or_later > 0 )
		back_call(now_or_later-1);
	else
		throw 66;
}

