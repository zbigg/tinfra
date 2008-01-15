#ifndef __tinfra_test_h__
#define __tinfra_test_h__

#include <string>

namespace tinfra {
namespace test {

class TempTestLocation {
public:
    explicit TempTestLocation(std::string const& name = "");
    ~TempTestLocation();
    
    std::string getPath() const;
        
    static void setTestResourcesDir(std::string const& x);
private:
    void init();
    std::string name_;
    std::string orig_pwd_;
    std::string tmp_path_;
};

void user_wait(const char* prompt = 0);

} } // end namespace tinfra::test

#endif
