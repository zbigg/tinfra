#include "multitype_map.h"

#include <iostream>
#include <string>
using namespace std;

int main()
{
    tinfra::multitype_map<string> m;
    m.put("b",5);
    m.put("a",2.4);
    m.put("a",string("Z"));
    cout << "m<int>(b)" << m.get<int>("b") << endl;
    cout << "m<float>(a)" << m.get<float>("a") << endl;
    cout << "m<string>(a)" << m.get<string>("a") << endl;
    return 0;
}
