//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_buffered_aio_adapter_h_included__
#define tinfra_buffered_aio_adapter_h_included__

#include "tinfra/aio.h"
#include "tinfra/parser.h"

#include <memory>

namespace tinfra {
namespace aio {

class buffered_aio_adapter: public tinfra::aio::listener {
    tinfra::parser& parser_;
public:
    buffered_aio_adapter(tinfra::parser& p);
    ~buffered_aio_adapter();
    
    stream* get_output_channel();
    
    // implementation of tinfra::aio::listener
    virtual void event(dispatcher& d, stream* c, int event);
    virtual void failure(dispatcher& d, stream* c, int error);
    
private:
    class buffer_impl;
    class reader_impl;
    class writer_impl;
    
    std::auto_ptr<reader_impl> reader_;
    std::auto_ptr<writer_impl> writer_;
};

} } // end of namespace tinfra::aio

#endif // tinfra_buffered_aio_adapter_h_included__

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++

