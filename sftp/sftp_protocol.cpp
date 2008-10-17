#include "tinfra/symbol.h"
#include "sftp_protocol.h"

#define IMPL_SYMBOL(a) const tinfra::symbol a(#a)

namespace sftp {
    
namespace S {
    IMPL_SYMBOL(length);
    IMPL_SYMBOL(type);
    IMPL_SYMBOL(request_id);
    
    IMPL_SYMBOL(version);
    IMPL_SYMBOL(extensions);
    
    IMPL_SYMBOL(valid_attribute_flags);
    IMPL_SYMBOL(size);
    IMPL_SYMBOL(allocation_size);
    IMPL_SYMBOL(owner);
    IMPL_SYMBOL(group);
    IMPL_SYMBOL(permissions);
    IMPL_SYMBOL(atime);
    IMPL_SYMBOL(atime_nseconds);
    IMPL_SYMBOL(createtime);
    IMPL_SYMBOL(createtime_nseconds);
    IMPL_SYMBOL(mtime);
    IMPL_SYMBOL(mtime_nseconds);
    IMPL_SYMBOL(ctime);
    IMPL_SYMBOL(ctime_nseconds);
    IMPL_SYMBOL(acl);
    IMPL_SYMBOL(attrib_bits);
    IMPL_SYMBOL(attrib_bits_valid);
    IMPL_SYMBOL(text_hint);
    IMPL_SYMBOL(mime_type);
    IMPL_SYMBOL(link_count);
    IMPL_SYMBOL(untranslated_name);
    IMPL_SYMBOL(extended_count);
    
    IMPL_SYMBOL(filename);
    IMPL_SYMBOL(desired_access);
    IMPL_SYMBOL(flags);
    IMPL_SYMBOL(attrs);
    IMPL_SYMBOL(path);
    IMPL_SYMBOL(handle);
    IMPL_SYMBOL(offset);
    IMPL_SYMBOL(data);
    
    IMPL_SYMBOL(oldpath);
    IMPL_SYMBOL(newpath);
    
    IMPL_SYMBOL(status_code);
    IMPL_SYMBOL(error_message);
    IMPL_SYMBOL(language_tag);
    
    IMPL_SYMBOL(end_of_file);
    IMPL_SYMBOL(elements);
}


} // end namespace sftp
