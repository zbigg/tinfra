//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra_sequence_parser_h__
#define __tinfra_sequence_parser_h__

#include "tinfra/tstring.h"

namespace tinfra {

template <typename T>
class sequence_parser {
public:
	virtual int   process_input(tinfra::tstring const& input) = 0;
	virtual void  eof(tinfra::tstring const& unparsed_input) = 0;
        virtual bool  get_result(T& r) = 0;
};

} // end namespace tinfra

#endif //__tinfra_parser_h__

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++
