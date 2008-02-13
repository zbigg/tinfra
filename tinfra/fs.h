#include <string>
#include <vector>

namespace tinfra {
namespace fs {

struct file_list_visitor {
    virtual void accept(const char* name) =0;
};
void list_files(const char* path, file_list_visitor& visitor);
void list_files(const char* path, std::vector<std::string>& result);
inline std::vector<std::string> list_files(const char* path) {
    std::vector<std::string> r;
    list_files(path, r);
    return r;
}
    
void copy(const char* src, const char* dest);

void cd(const char* dirname);
std::string pwd();
void mkdir(const char* name, bool create_parents = true);

void rm(const char* name);
void rmdir(const char* name);

void recursive_copy(const char* src, const char* dest);
void recursive_rm(const char* src);

// std::string inliners
inline
void list_files(std::string const& name, std::vector<std::string>& result) { return list_files(name.c_str(), result); }

inline 
void recursive_copy(std::string const& src, std::string const& dest) { return recursive_copy(src.c_str(), dest.c_str()); }

} }

