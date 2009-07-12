//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef tinfra_cmd_h_included
#define tinfra_cmd_h_included

#include <string>
#include <tinfra/tstring.h>

namespace tinfra {
namespace cmd {

class app {
public:
    app();
    virtual ~app();
    void               program_name(std::string const& p);
    std::string const& program_name() const;

    unsigned error_count() const;
    void fail(tstring const& msg);
    void warning(tstring const& msg);
    void silent_exception(tstring const& msg);
    void inform(tstring const& msg);
    void error(tstring const& msg);

    static app& get();
private:
    std::string program_name_;

    unsigned error_count_;
    unsigned warning_count_;
};

inline void warning(tstring const& m) { app::get().warning(m); }
inline void inform(tstring const& m) { app::get().inform(m); }
inline void fail(tstring const& m) { app::get().fail(m); }
inline void error(tstring const& m) { app::get().error(m); }

int main_wrapper(int argc, char* argv[],int (*real_main)(int,char*[]));

} // end of namespace cmd

inline tinfra::cmd::app& get_app() { return tinfra::cmd::app::get(); }
    
#define TINFRA_MAIN(a)                                   \
int main(int argc, char** argv)                          \
    { return tinfra::cmd::main_wrapper(argc, argv, a); } \
int TINFRA_MAIN_statement_enforement

} // end of namespace tinfra

#endif
