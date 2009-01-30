//
// prepends timestamp (and optionaly string) at start of each line 
//

#include <string>
#include <iostream>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

std::string date_format = "%b %d %T";
void stamp(std::ostream& out)
{
	char outstr[200];
	time_t now;
	struct tm *now_tm;
	
	now = time(NULL);
	now_tm = localtime(&now);
	if (now_tm == NULL) {
		return;
	}
	
	if (strftime(outstr, sizeof(outstr), date_format.c_str(), now_tm) == 0) {
		return;
	}
	out << outstr << ' ';
}

int main(int argc, char** argv)
{
	std::string synopsis;
	
	for( int i=1; i < argc; i++ ) {
		if( i > 1 ) synopsis += ' ';
		synopsis += argv[i];
	}
	
	std::string line;	
	while( std::getline(std::cin, line) ) {
		stamp(std::cout);
		if( synopsis.size() > 0 )
			std::cout << synopsis << ' ';
		std::cout << line << std::endl;
	}
	return 0;
}

