#include "tinfra/symbol.h"

#include "tinfra/symbol.h"

#define TINFRA_DECLARE_STRUCT template <typename F> void apply(F& field) const
#define FIELD(a) field(S::a, a)
#define DECL_SYMBOL(a) extern tinfra::symbol a;

namespace sftp {
    
namespace S {
    DECL_SYMBOL(length);
    DECL_SYMBOL(type);
    DECL_SYMBOL(request_id);
}


    struct packet_header {
        uint32  length;
        byte    type;
        uint32  request_id;
        
        TINFRA_DECLARE_STRUCT {
            FIELD(length);
            FIELD(type);
            FIELD(request_id);
        }
    };


}