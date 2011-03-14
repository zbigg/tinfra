#include <tinfra/csv.h>
#include <tinfra/cmd.h>
#include <iostream>
#include <vector>
#include <string>
#include <cassert>

const char delimiter = ';';

int csv_decompose(int argc, char** argv)
{
    tinfra::raw_csv_reader reader(std::cin, delimiter);
    tinfra::csv_raw_entry entry;
    
    assert(reader.fetch_next(entry));
    std::vector<std::string> headers(entry.begin(), entry.end());
    
    while( reader.fetch_next(entry) ) {
        for( size_t idx = 1; idx < entry.size(); ++idx ) {
            if( idx >= headers.size() )
                continue;
            std::cout << entry[0] << delimiter << headers[idx] << delimiter << entry[idx] << "\n";
        }
    }
    return 0;
}

TINFRA_MAIN(csv_decompose);
