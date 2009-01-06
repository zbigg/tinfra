#include "tinfra/csv.h"
#include "tinfra/cmd.h"

#include <iostream>
#include <iterator>
#include <sstream>

#ifdef BUILD_UNITTEST
#include "tinfra/test.h"
#endif

class csv_ostream {
    std::ostream& out_;
    
public:
    csv_ostream(std::ostream& out): out_(out) {}

    std::ostream& get_ostream() { return out_; }
    
    template <typename T>
    void operator()(T const& v) {
        (*this) << v;
    }
    
    void new_entry() {
        out_ << '\n';
    }
};

csv_ostream& operator <<(csv_ostream& out, tinfra::tstring const& v)
{
    tinfra::escape_csv(v, out.get_ostream());
    return out;
}

csv_ostream& operator <<(csv_ostream& out, std::string const& v)
{
    tinfra::escape_csv(v, out.get_ostream());
    return out;
}

csv_ostream& operator <<(csv_ostream& out, const char* v)
{
    tinfra::escape_csv(v, out.get_ostream());
    return out;
}

template <typename T>
csv_ostream& operator <<(csv_ostream& out, T const& v)
{
    std::ostringstream s; 
    s << v;
    tinfra::escape_csv(v.str(), out.get_ostream());
    return out;
}

template <typename T>
csv_ostream& operator <<(csv_ostream& out, std::vector<T> const& v)
{
    for( typename std::vector<T>::const_iterator i = v.begin(); i != v.end(); ++i ) {
        if( i != v.begin() )
            out.get_ostream() << ",";
        out << *i;
    }
    return out;
}

#include <iostream>
int importer_main(int argc, char** argv)
{
#ifdef BUILD_UNITTEST
    if( argc>1 && strcmp(argv[1], "--tests") == 0)
        return tinfra::test::test_main(argc,argv);
#endif
    char bubufre[512];
    std::cin.rdbuf()->pubsetbuf(bubufre, sizeof(bubufre));
    tinfra::raw_csv_reader reader(std::cin);
    tinfra::csv_raw_entry entry;
    csv_ostream formatter(std::cout);
    
    while( reader.fetch_next(entry) ) {
        formatter(entry);
        formatter.new_entry();
    }
    return 0;
}

TINFRA_MAIN(importer_main);

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++
