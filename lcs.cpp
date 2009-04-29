//
// This is a Longest common subsequence problem solution
// as described 
//    http://en.wikipedia.org/wiki/Longest_common_subsequence_problem
//
// this is _naive_, recursive approach without memoization.
// proof of concept, checking if i can still program some algorithm
// DONT TRY IT IN PRODUCTION
//

#include <vector>
#include <cassert>
#include <iostream>

namespace std {
template <typename T>
ostream& operator <<(ostream& s, std::vector<T> const& v)
{
	if( v.size() == 0 ) {
		return s << "{}";
	}
	s << "{";
	for(size_t i = 0; i < v.size(); ++i ) {
		if( i > 0 )
			s << ",";
		s << v[i];
	}
	s << "}";
	return s;
}
}

template <typename C>
typename C::value_type last(C const& c)
{
	assert( c.size() > 0 );
	return c[c.size()-1];
}

template <typename C>
size_t size(C const& c)
{
	return c.size();
}

template <typename C>
C cutlast(C const& c)
{
	assert( c.size() > 0 );
	return C(c.begin(), c.begin() + c.size() - 1);
}



template <typename S>
S lcs(S const& X, S const& Y)
{
	std::cout << "IN lcs X=" << X << " Y=" << Y <<"\n";
	if( size(X) == 0 || size(Y) == 0 )
		return S();
	
	if( last(X) == last(Y) ) {
		S tmp = lcs( cutlast(X), cutlast(Y) );
		tmp.push_back(last(X));
		return tmp;
	}
	
	const S a = lcs( X, cutlast(Y) );
	const S b = lcs( cutlast(X), Y );
	
	if( size(a) > size(b) )
		return a;
	else
		return b;
}

using std::string;
using std::vector;

typedef vector<string> seq;

void print_diff(seq const& a, seq const& b, seq const& z)
{
	using std::cout; 
	seq::const_iterator ia = a.begin();
	seq::const_iterator ib = b.begin();
	seq::const_iterator iz = z.begin();
	
	while( true ) {
		if( ia == a.end()  ) {
			assert( iz == z.end());
			while( ib != b.end() ) {
				cout << "+" << *ib++ << "\n";
			}
			break;
		} else if( ib == b.end() ) {
			assert( iz == z.end());
			while( ia != a.end() ) {
				cout << "-" << *ia++ << "\n";	
			}
			break;
		} else if( *ia != *iz ) {
			cout << "-" << *ia++ << "\n";
		} else if( *ib != *iz ) {
			cout << "+" << *ib++ << "\n";
		} else {
			
			assert(*ib == *iz);
			assert(*ia == *iz);
			cout << " " << *iz << "\n";
			ia++;
			ib++;
			iz++;
		}
		
	}
}

int main(int argc, char** argv)
{
	
	seq X;
	X.push_back("A");
	X.push_back("G");
	X.push_back("H");
	X.push_back("C");
	X.push_back("D");
	X.push_back("E");
	X.push_back("B");
	X.push_back("T");
	
	
	seq Y;
	Y.push_back("G");
	Y.push_back("H");
	Y.push_back("A");
	Y.push_back("C");
	Y.push_back("D");
	Y.push_back("E");
	Y.push_back("T");
	
	vector<string> z = lcs(X,Y);
	std::cout << "X = " << X << "\n";
	std::cout << "Y = " << Y << "\n";
	std::cout << "lcs(X,Y) = " << z << "\n";
	
	print_diff(X,Y,z);
}


