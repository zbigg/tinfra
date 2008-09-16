//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#include "tinfra/subprocess.h"
#include "tinfra/fmt.h"

#include <vector>
#include <string>
#include <stdexcept>
namespace tinfra {
    
static void read_file(tinfra::io::stream* s, std::string& data)
{
    char buf[1024];
    int r;
    while( ( r = s->read(buf, sizeof(buf))) > 0 ) {
        data.append(buf, r);
    }
}

std::string capture_command(std::string const& command)
{
    std::auto_ptr<subprocess> p = tinfra::subprocess::create();
                
    p->set_stdout_mode(subprocess::REDIRECT);
    
    std::string result;
    p->start(command.c_str());
    
    read_file(p->get_stdout(), result);
    
    p->wait();
    const int exit_code = p->get_exit_code();
    
    if( exit_code != 0 ) {
        throw std::runtime_error(fmt("command '%s' failed status $s") % command % exit_code);
    }
    
    return result;
}

}
