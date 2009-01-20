//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_parser_h__
#define tinfra_parser_h__

#include "tinfra/tstring.h"

namespace tinfra {

class parser {
public:
	virtual int   process_input(tinfra::tstring const& input) = 0;
	virtual void  eof(tinfra::tstring const& unparsed_input) = 0;


        virtual ~parser() {}
};

} // end namespace tinfra

#endif // tinfra_parser_h__

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++
