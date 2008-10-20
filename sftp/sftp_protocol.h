#ifndef __sftp_protocol_h__
#define __sftp_protocol_h__

#include "tinfra/symbol.h"
#include "tinfra/tinfra.h"
#include "rfc4251.h"


namespace sftp {

// types from rfc4251
using rfc4251::byte;
using rfc4251::boolean;
using rfc4251::uint32;
using rfc4251::uint64;
using rfc4251::string;
    
// specific SFTP types    
typedef unsigned short   uint16;
typedef signed long long int64;

struct extension_pair;


// TODO: fill_list
// is a list that fills up remaining message area
// when reading one should try to read elements as long
// as data are avilable
// when writing, just write elements without saying what is
// the size or where is the end (it will be packet end)

template <typename T>
class fill_list: public std::vector<T> {
public:
    fill_list(): 
        std::vector<T>() 
    {}
        
    fill_list(std::vector<T> const& other): 
        std::vector<T>(other) 
    {}
};

template <typename T>
class prefixed_list: public std::vector<T> {
public:
    typedef uint32 serialized_size_type;

    prefixed_list(): 
        std::vector<T>() 
    {}
        
    prefixed_list(std::vector<T> const& other): 
        std::vector<T>(other) 
    {}
};

//
// enums
//

enum packet_type {
    SSH_FXP_INIT       =  1,
    SSH_FXP_VERSION    =  2,
    SSH_FXP_OPEN       =  3,
    SSH_FXP_CLOSE      =  4,
    SSH_FXP_READ       =  5,
    SSH_FXP_WRITE      =  6,
    SSH_FXP_LSTAT      =  7,
    SSH_FXP_FSTAT      =  8,
    SSH_FXP_SETSTAT    =  9,
    SSH_FXP_FSETSTAT   =  10,
    SSH_FXP_OPENDIR    =  11,
    SSH_FXP_READDIR    =  12,
    SSH_FXP_REMOVE     =  13,
    SSH_FXP_MKDIR      =  14,
    SSH_FXP_RMDIR      =  15,
    SSH_FXP_REALPATH   =  16,
    SSH_FXP_STAT       =  17,
    SSH_FXP_RENAME     =  18,
    SSH_FXP_READLINK   =  19,
    SSH_FXP_LINK       =  21,
    SSH_FXP_BLOCK      =  22,
    SSH_FXP_UNBLOCK    =  23,
    
    SSH_FXP_STATUS     =  101,
    SSH_FXP_HANDLE     =  102,
    SSH_FXP_DATA       =  103,
    SSH_FXP_NAME       =  104,
    SSH_FXP_ATTRS      =  105,
     
    SSH_FXP_EXTENDED   =  200,
    SSH_FXP_EXTENDED_REPLY     =  201
};

enum fileattr {
    SSH_FILEXFER_ATTR_SIZE             =  0x00000001,
    SSH_FILEXFER_ATTR_UIDGID           =  0x00000002,
    SSH_FILEXFER_ATTR_PERMISSIONS      =  0x00000004,
    SSH_FILEXFER_ATTR_ACCESSTIME       =  0x00000008,
    SSH_FILEXFER_ATTR_ACMODTIME        =  0x00000008,
    SSH_FILEXFER_ATTR_CREATETIME       =  0x00000010,
    SSH_FILEXFER_ATTR_MODIFYTIME       =  0x00000020,
    SSH_FILEXFER_ATTR_ACL              =  0x00000040,
    SSH_FILEXFER_ATTR_OWNERGROUP       =  0x00000080,
    SSH_FILEXFER_ATTR_SUBSECOND_TIMES  =  0x00000100,
    SSH_FILEXFER_ATTR_BITS             =  0x00000200,
    SSH_FILEXFER_ATTR_ALLOCATION_SIZE  =  0x00000400,
    SSH_FILEXFER_ATTR_TEXT_HINT        =  0x00000800,
    SSH_FILEXFER_ATTR_MIME_TYPE        =  0x00001000,
    SSH_FILEXFER_ATTR_LINK_COUNT       =  0x00002000,
    SSH_FILEXFER_ATTR_UNTRANSLATED_NAME =  0x00004000,
    SSH_FILEXFER_ATTR_CTIME            =  0x00008000,
    SSH_FILEXFER_ATTR_EXTENDED         =  0x80000000 
};

enum filetype {
    SSH_FILEXFER_TYPE_REGULAR      =  1,
    SSH_FILEXFER_TYPE_DIRECTORY    =  2,
    SSH_FILEXFER_TYPE_SYMLINK      =  3,
    SSH_FILEXFER_TYPE_SPECIAL      =  4,
    SSH_FILEXFER_TYPE_UNKNOWN      =  5,
    SSH_FILEXFER_TYPE_SOCKET       =  6,
    SSH_FILEXFER_TYPE_CHAR_DEVICE  =  7,
    SSH_FILEXFER_TYPE_BLOCK_DEVICE =  8,
    SSH_FILEXFER_TYPE_FIFO         =  9
};

enum open_flags {
  SSH_FXF_READ                     = 0x00000001,
  SSH_FXF_WRITE                    = 0x00000002,
  SSH_FXF_APPEND                   = 0x00000004,
  SSH_FXF_CREAT                    = 0x00000008,
  SSH_FXF_TRUNC                    = 0x00000010,
  SSH_FXF_EXCL                     = 0x00000020
};

//
// symbol definitions
//

#define DECL_SYMBOL(a) extern const tinfra::symbol a;

namespace S {    
    DECL_SYMBOL(length);
    DECL_SYMBOL(type);
    DECL_SYMBOL(request_id);
    
    DECL_SYMBOL(version);
    DECL_SYMBOL(extensions);
    
    DECL_SYMBOL(valid_attribute_flags);
    DECL_SYMBOL(type);
    DECL_SYMBOL(size);
    DECL_SYMBOL(allocation_size);
    DECL_SYMBOL(uid);
    DECL_SYMBOL(gid);
    DECL_SYMBOL(owner);
    DECL_SYMBOL(group);
    DECL_SYMBOL(permissions);
    DECL_SYMBOL(atime);
    DECL_SYMBOL(atime_nseconds);
    DECL_SYMBOL(createtime);
    DECL_SYMBOL(createtime_nseconds);
    DECL_SYMBOL(mtime);
    DECL_SYMBOL(mtime_nseconds);
    DECL_SYMBOL(ctime);
    DECL_SYMBOL(ctime_nseconds);
    DECL_SYMBOL(acl);
    DECL_SYMBOL(attrib_bits);
    DECL_SYMBOL(attrib_bits_valid);
    DECL_SYMBOL(text_hint);
    DECL_SYMBOL(mime_type);
    DECL_SYMBOL(link_count);
    DECL_SYMBOL(untranslated_name);
    DECL_SYMBOL(extended_count);
    
    DECL_SYMBOL(filename);
    DECL_SYMBOL(longname);
    DECL_SYMBOL(desired_access);
    DECL_SYMBOL(flags);
    DECL_SYMBOL(attrs);
    DECL_SYMBOL(path);
    DECL_SYMBOL(handle);
    DECL_SYMBOL(offset);
    DECL_SYMBOL(data);
    
    DECL_SYMBOL(name);
    DECL_SYMBOL(oldpath);
    DECL_SYMBOL(newpath);
    
    DECL_SYMBOL(status_code);
    DECL_SYMBOL(error_message);
    DECL_SYMBOL(language_tag);
    
    DECL_SYMBOL(end_of_file);
    DECL_SYMBOL(elements);
};

//
// compund types
//

#define TINFRA_DECLARE_STRUCT template <typename F> void apply(F& field) const
#define FIELD(a) field(S::a, a)

struct extension_pair {
    string name;
    string data;
    
    TINFRA_DECLARE_STRUCT {
        FIELD(name);
        FIELD(data);
    }
};

struct attr {
    uint32 valid_attribute_flags;
    //byte   type;
    uint64 size;
    //uint64 allocation_size;
    uint32 uid;
    uint32 gid;
    //string owner;
    //string group;
    uint32 permissions;
    uint32  atime;
    //uint32 atime_nseconds;
    //int64  createtime;
    //uint32 createtime_nseconds;
    uint32  mtime;
    //uint32 mtime_nseconds;
    //int64  ctime;
    //uint32 ctime_nseconds;
    //string acl;
    //uint32 attrib_bits;
    //uint32 attrib_bits_valid;
    //byte   text_hint;
    //string mime_type;
    //uint32 link_count;
    //string untranslated_name;
    prefixed_list<extension_pair> extensions;
    
    TINFRA_DECLARE_STRUCT {
                                                         FIELD(valid_attribute_flags);
        //                                                 FIELD(type);

        if ( present(SSH_FILEXFER_ATTR_SIZE) )           FIELD(size);
        //if ( present(SSH_FILEXFER_ATTR_ALLOCATION_SIZE)) FIELD(allocation_size);
        if ( present(SSH_FILEXFER_ATTR_UIDGID) )         FIELD(uid);
        if ( present(SSH_FILEXFER_ATTR_UIDGID) )         FIELD(gid);
        //if ( present(SSH_FILEXFER_ATTR_OWNERGROUP) )     FIELD(owner);
        //if ( present(SSH_FILEXFER_ATTR_OWNERGROUP) )     FIELD(group);
        if ( present(SSH_FILEXFER_ATTR_PERMISSIONS) )    FIELD(permissions);
        if ( present(SSH_FILEXFER_ATTR_ACMODTIME) )     FIELD(atime);
        //if ( present(SSH_FILEXFER_ATTR_SUBSECOND_TIMES)) FIELD(atime_nseconds);
        //if ( present(SSH_FILEXFER_ATTR_CREATETIME) )     FIELD(createtime);
        //if ( present(SSH_FILEXFER_ATTR_SUBSECOND_TIMES) )FIELD(createtime_nseconds);
        if ( present(SSH_FILEXFER_ATTR_ACMODTIME) )     FIELD(mtime);
        //if ( present(SSH_FILEXFER_ATTR_SUBSECOND_TIMES) )FIELD(mtime_nseconds);
        //if ( present(SSH_FILEXFER_ATTR_CTIME) )          FIELD(ctime);
        //if ( present(SSH_FILEXFER_ATTR_SUBSECOND_TIMES) )FIELD(ctime_nseconds);
        //if ( present(SSH_FILEXFER_ATTR_ACL) )            FIELD(acl);
        //if ( present(SSH_FILEXFER_ATTR_BITS) )           FIELD(attrib_bits);
        //if ( present(SSH_FILEXFER_ATTR_BITS) )           FIELD(attrib_bits_valid);
        //if ( present(SSH_FILEXFER_ATTR_TEXT_HINT) )      FIELD(text_hint);
        //if ( present(SSH_FILEXFER_ATTR_MIME_TYPE) )      FIELD(mime_type);
        //if ( present(SSH_FILEXFER_ATTR_LINK_COUNT) )     FIELD(link_count);
        //if ( present(SSH_FILEXFER_ATTR_UNTRANSLATED_NAME) )     FIELD(untranslated_name);
        if ( present(SSH_FILEXFER_ATTR_EXTENDED) )       FIELD(extensions);

                                                         
    }
    
    bool present(fileattr field) const {
        return (valid_attribute_flags & field) != 0;
    }
};

//
// requests packets
//

struct packet_header {
    uint32  length;
    byte    type;    
    
    TINFRA_DECLARE_STRUCT {
        FIELD(length);
        FIELD(type);
    }
};

struct init_packet {
    static const packet_type type = SSH_FXP_INIT;
    
    uint32 version;
    
    TINFRA_DECLARE_STRUCT {
        FIELD(version);
    }
};

struct version_packet {
    static const packet_type type = SSH_FXP_VERSION;
    
    uint32 version;
    fill_list<extension_pair> extensions;
    
    TINFRA_DECLARE_STRUCT {
        FIELD(version);
        FIELD(extensions);
    }
};

struct open_packet {
    static const packet_type type = SSH_FXP_OPEN;
    
    uint32 request_id;
    string filename; // UTF
    //uint32 desired_access;
    uint32 flags;
    attr   attrs;  // ???
    
    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(filename);
        //FIELD(desired_access);
        FIELD(flags);
        FIELD(attrs);
    }
};

struct open_dir_packet {
    static const packet_type type = SSH_FXP_OPENDIR;
    
    uint32 request_id;
    string path; // UTF
    
    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(path);
    }
};

struct close_packet {
    static const packet_type type = SSH_FXP_CLOSE;
    
    uint32 request_id;
    string handle;
    
    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(handle);
    }
};

struct read_packet {
    static const packet_type type = SSH_FXP_READ;
    
    uint32 request_id;
    string handle;
    uint64 offset;
    uint32 length;
    
    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(handle);
        FIELD(offset);
        FIELD(length);
    }
};

struct read_dir_packet {
    static const packet_type type = SSH_FXP_READDIR;
    
    uint32 request_id;
    string handle;
    
    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(handle);
    }
};

struct write_packet {
    static const packet_type type = SSH_FXP_WRITE;
    
    uint32 request_id;
    string handle;
    uint64 offset;
    string data;

    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(handle);
        FIELD(offset);
        FIELD(data);
    }
};

struct remove_packet {
    static const packet_type type = SSH_FXP_REMOVE;
    
    uint32 request_id;
    string filename;
    
    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(filename);
    }
};

struct rename_packet {
    static const packet_type type = SSH_FXP_RENAME;
    
    uint32 request_id;
    string oldpath;
    string newpath;
    //uint32 flags;

    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(oldpath);
        FIELD(newpath);
        //FIELD(flags);
    }
};

struct mkdir_packet {
    static const packet_type type = SSH_FXP_MKDIR;
    
    uint32 request_id;
    string path;
    //attr   attrs;

    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(path);
        //FIELD(attrs);
    }
};

struct rmdir_packet {
    static const packet_type type = SSH_FXP_RMDIR;
    
    uint32 request_id;
    string path;
    
    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(path);
    }
};

struct stat_packet {
    static const packet_type type = SSH_FXP_STAT;
    
    uint32 request_id;
    string path;
    //uint32 flags;
    
    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(path);
        //FIELD(flags);
    }
};

struct set_stat_packet {
    static const packet_type type = SSH_FXP_SETSTAT;
    
    uint32 request_id;
    string path;
    attr   attrs;

    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(path);
        FIELD(attrs);
    }
};

//
// response packets
//

struct status_packet {    
    static const packet_type type = SSH_FXP_STATUS;
    
    uint32 request_id;
    uint32 status_code;
    //string error_message;
    //string language_tag;
    
    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(status_code);
        //FIELD(error_message);
        //FIELD(language_tag);
    }
};

struct handle_packet {
    static const packet_type type = SSH_FXP_HANDLE;
    
    uint32 request_id;
    string handle;
    
    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(handle);
    }
};

struct data_packet {
    static const packet_type type = SSH_FXP_DATA;
    
    uint32 request_id;
    string data;
    //bool   end_of_file; // TODO this is optional, current
                        //      infra has no idea how to support
                        //      this
    
    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(data);
        //FIELD(end_of_file);
    }
};

struct name_element {
    string filename;
    string longname;
    attr   attrs;
    
    TINFRA_DECLARE_STRUCT {
        FIELD(filename);
        FIELD(longname);
        FIELD(attrs);
    }
};

struct name_packet {
    static const packet_type type = SSH_FXP_NAME;
    
    uint32 request_id;
    // WARNING custom encoding see 9.4 for encoding
    prefixed_list<name_element> elements; 
    
    //bool   end_of_file; // WARNING: optional
    
    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(elements);
        //FIELD(end_of_file);
    }
};

struct attrs_packet {
    static const packet_type type = SSH_FXP_ATTRS;
    
    uint32 request_id;
    attr   attrs;
    
    TINFRA_DECLARE_STRUCT {
        FIELD(request_id);
        FIELD(attrs);
    }
};

//
// readers and writers
//

class reader: public rfc4251::reader {
public:
    reader(const char* data, int length):
        rfc4251::reader(data, length)
    {
    }
    
    using rfc4251::reader::operator();
    
    void operator()(tinfra::symbol const&, uint16& r) { 
        uint16 const* tnext = reinterpret_cast<uint16 const*>(next);
        advance(2,"uint16");
        r = ntohs( tnext[0] );
    }
    
    void operator()(tinfra::symbol const&, int64& r) { 
        uint32 ur = read_uint64();
        r = static_cast<int64>(ur);
    }
    
    void operator()(tinfra::symbol const&, extension_pair& r) {
        read_string(r.name);
        read_string(r.data);
    }
    
    template <typename T>
    void operator()(tinfra::symbol const&, fill_list<T> & r) {
        while( true ) {
            try {
                T instance;
                tinfra::tt_mutate(instance, *this);
                r.push_back(instance);
            } catch( tinfra::io::would_block& e) {
                break;
            }
        }
    }
    template <typename T>
    void operator()(tinfra::symbol const&, prefixed_list<T> & r) {
        uint32 size = read_uint32();        
        r.reserve(size);
        for( uint32 i = 0; i != size; ++i ) {
            T instance;
            tinfra::tt_mutate(instance, *this);
            r.push_back(instance);
        }
    }
    template <typename T>
    void managed_struct(T& v, tinfra::symbol const& s)
    {    
        tinfra::tt_mutate<T>(v, *this);
    }
};

class writer: public rfc4251::writer {
public:
    writer(std::string& buffer):
        rfc4251::writer(buffer)
    {
    }
    
    using rfc4251::writer::operator();
    
    void operator()(tinfra::symbol const&, uint16 v) { 
        unsigned short nv = htons(v);
        write(nv);
    }
    
    void operator()(tinfra::symbol const&, int64 v)    { 
        write_uint64(static_cast<uint64>(v));
    }
    
    void operator()(tinfra::symbol const&, extension_pair const& v) {
        write_string(v.name);
        write_string(v.data);
    }

    template <typename T>
    void operator()(tinfra::symbol const&, fill_list<T> const& v) {
        for( typename fill_list<T>::const_iterator i = v.begin(); i != v.end(); ++i ) {
            tinfra::tt_process(*i, *this);
        }
    }
    
    template <typename T>
    void operator()(tinfra::symbol const&, prefixed_list<T> const& v) {    
        write_uint32(v.size());
        
        for( typename fill_list<T>::const_iterator i = v.begin(); i != v.end(); ++i ) {
            tinfra::tt_process(*i, *this);
        }
    }
    
    template <typename T>
    void managed_struct(T const& v, tinfra::symbol const& s)
    {    
        tinfra::tt_process<T>(v, *this);
    }
};


}

#define TINFRA_STRUCT(a) namespace tinfra { template <> class TypeTraits<a>: public tinfra::ManagedStruct<a> {}; }

TINFRA_STRUCT(sftp::packet_header);
TINFRA_STRUCT(sftp::init_packet);
TINFRA_STRUCT(sftp::version_packet);
TINFRA_STRUCT(sftp::status_packet);
TINFRA_STRUCT(sftp::attrs_packet);
TINFRA_STRUCT(sftp::name_element);

TINFRA_STRUCT(sftp::attr);
#endif // end __sftp_protocol_h__
