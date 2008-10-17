#include "tinfra/symbol.h"
#include "sftp_protocol.h"

#define INST_SYMBOL(a) tinfra::symbol a(#a)

namespace sftp {
    
namespace S {
    INST_SYMBOL(length);
    INST_SYMBOL(type);
    INST_SYMBOL(request_id);
}



}