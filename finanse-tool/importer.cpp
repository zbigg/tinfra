#include "tinfra/csv.h"
#include "tinfra/cmd.h"

#include <iostream>
#include <iterator>

#ifdef BUILD_UNITTEST
#include "tinfra/test.h"
#endif

void csv_out(std::ostream& out, tinfra::tstring const& value)
{
    bool need_quote = false;
    if( value.find_first_of(",\r\n") != tinfra::tstring::npos ) {
        need_quote = true;
    }    
    if( need_quote )
        out << '"' << value << '"';
    else
        out << value;
}
namespace std {
template <typename T>
std::ostream& operator <<(std::ostream& out, std::vector<T> const& v)
{
    out << "[";    
    for( typename std::vector<T>::const_iterator i = v.begin(); i != v.end(); ++i ) {
        if( i != v.begin() )
            out << ", ";
        csv_out(out,*i);
    }
    return out << "]";
}
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
    while( reader.fetch_next(entry) ) {
        std::cout << "readed " << entry << std::endl;
    }
    return 0;
}

TINFRA_MAIN(importer_main);

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++
