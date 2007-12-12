#ifndef __tinfra_cmd_h__
#define __tinfra_cmd_h__

#include <string>

namespace tinfra {
namespace cmd {

class app {
public:
    app();
    virtual ~app();
    void               program_name(std::string const& p);
    std::string const& program_name() const;

    unsigned error_count() const;
    void fail(std::string const& msg);
    void warning(std::string const& msg);
    void inform(std::string const& msg);
    void error(std::string const& msg);

    static app& get();
private:
    std::string program_name_;

    unsigned error_count_;
    unsigned warning_count_;
};

int main(int argc, char* argv[],int (*real_main)(int,char*[]));

} } // end of namespace tinfra::cmd

#endif
