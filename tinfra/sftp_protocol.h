//
// Copyright (C) 2008 Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __tinfra__sftp_protocol_h__
#define __tinfra__sftp_protocol_h__

#include "tinfra/symbol.h"
#include "tinfra/typeinfo.h"
#include "tinfra/standards/rfc4251.h"

namespace tinfra {
namespace sftp {

// types from rfc4251
using tinfra::rfc4251::byte;
using tinfra::rfc4251::uint32;
using tinfra::rfc4251::uint64;
using tinfra::rfc4251::string;
    
// specific SFTP types
    
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
    SSH_FILEXFER_TYPE_REGULAR      = 1,
    SSH_FILEXFER_TYPE_DIRECTORY    = 2,
    SSH_FILEXFER_TYPE_SYMLINK      = 3,
    SSH_FILEXFER_TYPE_SPECIAL      = 4,
    SSH_FILEXFER_TYPE_UNKNOWN      = 5,
    SSH_FILEXFER_TYPE_SOCKET       = 6,
    SSH_FILEXFER_TYPE_CHAR_DEVICE  = 7,
    SSH_FILEXFER_TYPE_BLOCK_DEVICE = 8,
    SSH_FILEXFER_TYPE_FIFO         = 9
};

enum open_flags {
    SSH_FXF_READ                   = 0x0001,
    SSH_FXF_WRITE                  = 0x0002,
    SSH_FXF_APPEND                 = 0x0004,
    SSH_FXF_CREAT                  = 0x0008,
    SSH_FXF_TRUNC                  = 0x0010,
    SSH_FXF_EXCL                   = 0x0020
};

enum status_code {
    SSH_FX_OK                      = 0,
    SSH_FX_EOF                     = 1,
    SSH_FX_NO_SUCH_FILE            = 2,
    SSH_FX_PERMISSION_DENIED       = 3,
    SSH_FX_FAILURE                 = 4,
    SSH_FX_BAD_MESSAGE             = 5,
    SSH_FX_NO_CONNECTION           = 6,
    SSH_FX_CONNECTION_LOST         = 7,
    SSH_FX_OP_UNSUPPORTED          = 8
};

//
// symbol definitions
//

//
// compund types
//

#define TINFRA_DECLARE_STRUCT template <typename F> void apply(F& f) const

struct extension_pair {
    string name;
    string data;
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(name);
        TINFRA_MO_FIELD(data);
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
                                                         TINFRA_MO_FIELD(valid_attribute_flags);
        //                                                 TINFRA_MO_FIELD(type);

        if ( present(SSH_FILEXFER_ATTR_SIZE) )           TINFRA_MO_FIELD(size);
        //if ( present(SSH_FILEXFER_ATTR_ALLOCATION_SIZE)) TINFRA_MO_FIELD(allocation_size);
        if ( present(SSH_FILEXFER_ATTR_UIDGID) )         TINFRA_MO_FIELD(uid);
        if ( present(SSH_FILEXFER_ATTR_UIDGID) )         TINFRA_MO_FIELD(gid);
        //if ( present(SSH_FILEXFER_ATTR_OWNERGROUP) )     TINFRA_MO_FIELD(owner);
        //if ( present(SSH_FILEXFER_ATTR_OWNERGROUP) )     TINFRA_MO_FIELD(group);
        if ( present(SSH_FILEXFER_ATTR_PERMISSIONS) )    TINFRA_MO_FIELD(permissions);
        if ( present(SSH_FILEXFER_ATTR_ACMODTIME) )     TINFRA_MO_FIELD(atime);
        //if ( present(SSH_FILEXFER_ATTR_SUBSECOND_TIMES)) TINFRA_MO_FIELD(atime_nseconds);
        //if ( present(SSH_FILEXFER_ATTR_CREATETIME) )     TINFRA_MO_FIELD(createtime);
        //if ( present(SSH_FILEXFER_ATTR_SUBSECOND_TIMES) )TINFRA_MO_FIELD(createtime_nseconds);
        if ( present(SSH_FILEXFER_ATTR_ACMODTIME) )     TINFRA_MO_FIELD(mtime);
        //if ( present(SSH_FILEXFER_ATTR_SUBSECOND_TIMES) )TINFRA_MO_FIELD(mtime_nseconds);
        //if ( present(SSH_FILEXFER_ATTR_CTIME) )          TINFRA_MO_FIELD(ctime);
        //if ( present(SSH_FILEXFER_ATTR_SUBSECOND_TIMES) )TINFRA_MO_FIELD(ctime_nseconds);
        //if ( present(SSH_FILEXFER_ATTR_ACL) )            TINFRA_MO_FIELD(acl);
        //if ( present(SSH_FILEXFER_ATTR_BITS) )           TINFRA_MO_FIELD(attrib_bits);
        //if ( present(SSH_FILEXFER_ATTR_BITS) )           TINFRA_MO_FIELD(attrib_bits_valid);
        //if ( present(SSH_FILEXFER_ATTR_TEXT_HINT) )      TINFRA_MO_FIELD(text_hint);
        //if ( present(SSH_FILEXFER_ATTR_MIME_TYPE) )      TINFRA_MO_FIELD(mime_type);
        //if ( present(SSH_FILEXFER_ATTR_LINK_COUNT) )     TINFRA_MO_FIELD(link_count);
        //if ( present(SSH_FILEXFER_ATTR_UNTRANSLATED_NAME) )     TINFRA_MO_FIELD(untranslated_name);
        if ( present(SSH_FILEXFER_ATTR_EXTENDED) )       TINFRA_MO_FIELD(extensions);

                                                         
    }
    
    bool present(fileattr TINFRA_MO_FIELD) const {
        return (valid_attribute_flags & TINFRA_MO_FIELD) != 0;
    }
};

//
// requests packets
//

struct packet_header {
    uint32  length;
    byte    type;   	 
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(length);
        TINFRA_MO_FIELD(type);
    }
};

struct init_packet {
    static const packet_type type = SSH_FXP_INIT;
    
    uint32 version;
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(version);
    }
};

struct version_packet {
    static const packet_type type = SSH_FXP_VERSION;
    
    uint32 version;
    fill_list<extension_pair> extensions;
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(version);
        TINFRA_MO_FIELD(extensions);
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
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(filename);
        //TINFRA_MO_FIELD(desired_access);
        TINFRA_MO_FIELD(flags);
        TINFRA_MO_FIELD(attrs);
    }
};

struct open_dir_packet {
    static const packet_type type = SSH_FXP_OPENDIR;
    
    uint32 request_id;
    string path; // UTF
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(path);
    }
};

struct close_packet {
    static const packet_type type = SSH_FXP_CLOSE;
    
    uint32 request_id;
    string handle;
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(handle);
    }
};

struct read_packet {
    static const packet_type type = SSH_FXP_READ;
    
    uint32 request_id;
    string handle;
    uint64 offset;
    uint32 length;
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(handle);
        TINFRA_MO_FIELD(offset);
        TINFRA_MO_FIELD(length);
    }
};

struct read_dir_packet {
    static const packet_type type = SSH_FXP_READDIR;
    
    uint32 request_id;
    string handle;
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(handle);
    }
};

struct write_packet {
    static const packet_type type = SSH_FXP_WRITE;
    
    uint32 request_id;
    string handle;
    uint64 offset;
    string data;

    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(handle);
        TINFRA_MO_FIELD(offset);
        TINFRA_MO_FIELD(data);
    }
};

struct remove_packet {
    static const packet_type type = SSH_FXP_REMOVE;
    
    uint32 request_id;
    string filename;
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(filename);
    }
};

struct rename_packet {
    static const packet_type type = SSH_FXP_RENAME;
    
    uint32 request_id;
    string oldpath;
    string newpath;
    //uint32 flags;

    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(oldpath);
        TINFRA_MO_FIELD(newpath);
        //TINFRA_MO_FIELD(flags);
    }
};

struct mkdir_packet {
    static const packet_type type = SSH_FXP_MKDIR;
    
    uint32 request_id;
    string path;
    //attr   attrs;

    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(path);
        //TINFRA_MO_FIELD(attrs);
    }
};

struct rmdir_packet {
    static const packet_type type = SSH_FXP_RMDIR;
    
    uint32 request_id;
    string path;
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(path);
    }
};

struct stat_packet {
    static const packet_type type = SSH_FXP_STAT;
    
    uint32 request_id;
    string path;
    //uint32 flags;
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(path);
        //TINFRA_MO_FIELD(flags);
    }
};

struct fstat_packet {
    static const packet_type type = SSH_FXP_FSTAT;
    
    uint32 request_id;
    string handle;
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(handle);
    }
};
struct set_stat_packet {
    static const packet_type type = SSH_FXP_SETSTAT;
    
    uint32 request_id;
    string path;
    attr   attrs;

    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(path);
        TINFRA_MO_FIELD(attrs);
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
    ///string language_tag;
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(status_code);
        //TINFRA_MO_FIELD(error_message);
        //TINFRA_MO_FIELD(language_tag);
    }
};

struct handle_packet {
    static const packet_type type = SSH_FXP_HANDLE;
    
    uint32 request_id;
    string handle;
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(handle);
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
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(data);
        //TINFRA_MO_FIELD(end_of_file);
    }
};

struct name_element {
    string filename;
    string longname;
    attr   attrs;
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(filename);
        TINFRA_MO_FIELD(longname);
        TINFRA_MO_FIELD(attrs);
    }
};

struct name_packet {
    static const packet_type type = SSH_FXP_NAME;
    
    uint32 request_id;
    // WARNING custom encoding see 9.4 for encoding
    prefixed_list<name_element> elements; 
    
    //bool   end_of_file; // WARNING: optional
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(elements);
        //TINFRA_MO_FIELD(end_of_file);
    }
};

struct attrs_packet {
    static const packet_type type = SSH_FXP_ATTRS;
    
    uint32 request_id;
    attr   attrs;
    
    TINFRA_DECLARE_STRUCT {
        TINFRA_MO_FIELD(request_id);
        TINFRA_MO_FIELD(attrs);
    }
};

//
// readers and writers
//

class reader: public tinfra::rfc4251::reader {
public:
    reader(const char* data, int length):
        tinfra::rfc4251::reader(data, length)
    {
    }
    
    using tinfra::rfc4251::reader::operator();
    
    /*
    void operator()(tinfra::symbol const&, uint16& r) { 
        uint16 const* tnext = reinterpret_cast<uint16 const*>(next);
        advance(2,"uint16");
        r = ntohs( tnext[0] );
    }
    
    void operator()(tinfra::symbol const&, int64& r) { 
        uint64 ur = read_uint64();
        r = static_cast<int64>(ur);
    }
    */
    
    void operator()(const char*, extension_pair& r) {
        read_string(r.name);
        read_string(r.data);
    }    
    
    template <typename T>
    void operator()(const char*, fill_list<T> & r) {
        while( true ) {
            try {
                T instance;
                tinfra::mo_mutate(instance, *this);
                r.push_back(instance);
            } catch( tinfra::io::would_block& e) {
                break;
            }
        }
    }
    template <typename T>
    void operator()(const char*, prefixed_list<T> & r) {
        uint32 size = read_uint32();        
        r.reserve(size);
        for( uint32 i = 0; i != size; ++i ) {
            T instance;
            tinfra::mo_mutate(instance, *this);
            r.push_back(instance);
        }
    }
    template <typename T>
    void mstruct(const char* s, T& v)
    {    
        tinfra::mo_mutate<T>(v, *this);
    }
};

class writer: public tinfra::rfc4251::writer {
public:
    writer(std::string& buffer):
        tinfra::rfc4251::writer(buffer)
    {
    }
    
    using tinfra::rfc4251::writer::operator();
    
    /*
    void operator()(tinfra::symbol const&, uint16 v) { 
        unsigned short nv = htons(v);
        write(nv);
    }
    
    void operator()(tinfra::symbol const&, int64 v)    { 
        write_uint64(static_cast<uint64>(v));
    }
    */
    void operator()(const char*, extension_pair const& v) {
        write_string(v.name);
        write_string(v.data);
    }

    template <typename T>
    void operator()(const char*, fill_list<T> const& v) {
        for( typename fill_list<T>::const_iterator i = v.begin(); i != v.end(); ++i ) {
            tinfra::mo_process(*i, *this);
        }
    }
    
    template <typename T>
    void operator()(const char*, prefixed_list<T> const& v) {    
        write_uint32(v.size());
        
        for( typename fill_list<T>::const_iterator i = v.begin(); i != v.end(); ++i ) {
            tinfra::process("", *i, *this);
        }
    }
    
    template <typename T>
    void mstruct(const char* s, T const& v)
    {    
        tinfra::mo_process<T>(v, *this);
    }
};


} } // end namespace tinfra::sftp

#define TINFRA_STRUCT(a) namespace tinfra { template <> class mo_traits<a>: public tinfra::struct_mo_traits<a> {}; }

TINFRA_STRUCT(tinfra::sftp::packet_header);
TINFRA_STRUCT(tinfra::sftp::init_packet);
TINFRA_STRUCT(tinfra::sftp::version_packet);
TINFRA_STRUCT(tinfra::sftp::status_packet);
TINFRA_STRUCT(tinfra::sftp::attrs_packet);
TINFRA_STRUCT(tinfra::sftp::name_element);
TINFRA_STRUCT(tinfra::sftp::stat_packet);
TINFRA_STRUCT(tinfra::sftp::fstat_packet);
TINFRA_STRUCT(tinfra::sftp::read_packet);
TINFRA_STRUCT(tinfra::sftp::write_packet);
TINFRA_STRUCT(tinfra::sftp::open_packet);
TINFRA_STRUCT(tinfra::sftp::open_dir_packet);
TINFRA_STRUCT(tinfra::sftp::close_packet);
TINFRA_STRUCT(tinfra::sftp::read_dir_packet);

TINFRA_STRUCT(sftp::attr);

#endif // end __sftp_protocol_h__
