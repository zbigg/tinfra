//
// Copyright (c) 2011, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "platform.h"

#include "tinfra/text.h"

namespace tinfra {

static bool readline(tinfra::input_stream& in, std::string& result)
{
    int readed = 0;
    result.clear();
    while( true ) {
        char c;
        const int r = in.read(&c, 1);
        if( r == 0 ) {
            break;
        }

        readed += 1;
        result.append(1, c);
        
        if( c== '\n' )
            break;
    }
    return readed != 0;
}


line_reader::line_reader(tinfra::input_stream& in): in(in)
{
}
line_reader::~line_reader()
{
}
	
bool line_reader::fetch_next(std::string& out)
{
	return readline(this->in, out);
}

} // end namespace tinfra
