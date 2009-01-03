#include "tinfra/csv.h"
#include "tinfra/cmd.h"

#include <iostream>
#include <iterator>

#ifdef BUILD_UNITTEST
#include "tinfra/test.h"
#endif

namespace std {
template <typename T>
std::ostream& operator <<(std::ostream& out, std::vector<T> const& v)
{
    out << "[";    
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(out, ", "));
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
    tinfra::raw_csv_reader reader(std::cin);
    tinfra::csv_raw_entry entry;
    while( reader.fetch_next(entry) ) {
        std::cerr << "readed " << entry << std::endl;
    }
    return 0;
}

TINFRA_MAIN(importer_main);

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++
