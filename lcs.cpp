//
// This is a Longest common subsequence problem solution
// as described 
//    http://en.wikipedia.org/wiki/Longest_common_subsequence_problem
//
// this is _naive_, recursive approach without memoization.
// proof of concept, checking if i can still program some algorithm
// DONT TRY IT IN PRODUCTION
//
// updates:
//   2012-07-18, API proposition added (update_info, find_updates)
//               new www resource, good discussion of optimization
//               strategies: http://wordaligned.org/articles/longest-common-subsequence
//               SO thread: http://stackoverflow.com/questions/3659032/efficient-longest-common-subsequence-algorithm-library


#include <vector>
#include <cassert>
#include <iostream>

//
// example API for LCS
//

enum update_type {
    ADD,
    DELETE,
    UPDATE
};

template <typename Target>
struct update_info {
    update_type type; // update type
    int        idx; // index in previous_list 
    Target*    old_value; // old value (valid if type in DELETE, UPDATE)
    Target*    new_value; // new value (valid if type in DELETE, UPDATE)
};

template <typename T>
std::vector< update_info<T> > find_updates(std::vector<T> const previous_list, std::vector<T> const updated_list);

/// find_updates
/// 
/// for each update (delete, update, new_value)
/// call f(Update<T> update)
///
/// Updates are sorted by occurence.
/// 
/// Example:
/// vector<A> new_list = get_some_list()
/// find_updates(this->current_list, new_list, [&](update_info<A> const& upd) {
///     int update_offset = 0; // needed to synchronize original and modified
///                            // indexes; upd.idx refer to original index
///     switch( upd.type ) {
///     case ADD:
///         some_ui.insert_item(upd.idx + update_offset, construct_gui_item(upd.new_value))
///         update_offset += 1;
///         break;
///     case DELETE:
///         some_ui.delete_item(upd.idx + update_offset)
///         update_offset -= 1;
///         break;
///     case UPDATE:
///         some_ui.update_item(upd.idx + update_offset, construct_gui_item(upd.new_value))
///         break;
///     }
/// });
///
/// this->current_list = new_list;

template <typename T, typename Functor f>
void find_updates(std::vector<T> const previous_list, std::vector<T> const& updated_list, Functor& f);

template <typename T, typename Container>
struct updater {
    Container& target;
    int update_offset;
    
    updater(Container& target): 
        target(target),
        update_offset(0) 
    {}
    
    void operator()(update_info<T> const& ui)
    {
        typename Container::iterator iter = this->target.begin();
        const size_t allowed_offset = (upd.type == ADD) 
            ? this->target.size() : 
              this->target.size()-1;
              
        assert(ui.idx + this-> update_offset <= allowed_offset);
        advance(iter, upd.idx + this->update_offset);
        
        switch( ui.type ) {
        case ADD:
            this->target.insert(iter);
            this->update_offset += 1;
            break;
        case DELETE:
            this->target.erase(iter);            
            this->update_offset -= 1;
            break;
        case UPDATE:
            *iter = ui.new_value;
            break;
        }
    }
};

template <typename T, typename Container f>
void apply_updates(std::vector<T> const previous_list, std::vector<T> const& updated_list, Container& target)
{
    updater<T,C> upd_handler(target);
    find_updates(previous_list, updated_list, upd_handler)
}

//
// LCS implementation
//
//   helpers

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


