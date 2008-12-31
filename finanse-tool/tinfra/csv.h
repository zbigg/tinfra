//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_csv_h__
#define __tinfra_csv_h__

#include <vector>
#include <string>
#include <istream>
#include "tinfra/generator.h"
#include "tinfra/tstring.h"

namespace tinfra {

typedef std::vector<tstring> csv_raw_entry;
class raw_csv_reader: public generator_impl<csv_reader, csv_raw_entry> {
    raw_csv_reader(std::istream& byte_source, char separator = ',');
protected:
    bool fetch_next(csv_raw_entry&);
private:
    std::istream byte_source_;
    string_pool  memory_pool_;
    char separator_;
};
    
} // end namespace tinfra

#endif // __tinfra_csv_h__

