//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#include "tinfra/adaptable.h"

#include "tinfra/shared_ptr.h"
#include <string>
#include <vector>

#include <unittest++/UnitTest++.h>

SUITE(tinfra) {

    struct ILabeled {
        virtual std::string label() const = 0;
    };
    
    struct IParent {
        typedef std::vector< tinfra::shared_ptr<tinfra::adaptable> > children_list_t;
        
        virtual int             children_count() const = 0;
        virtual children_list_t get_children() const = 0;
    };
    
    class Item: public tinfra::generic_adaptable,
                public ILabeled
    {
    public:
        Item(std::string const& name):
            name_(name)
        {
            tinfra_adapter.add<ILabeled>(*this);
        }
        
        std::string label() const
        {
            return name_;
        }
        
    private:
        std::string name_;
    };
    
    class Folder: public Item,
                  public IParent
    {
    public:
        Folder(std::string const& name):
            Item(name)
        {
            tinfra_adapter.add<IParent>(*this);
        }
        
        int             children_count() const
        {
            return 0;
        }
        IParent::children_list_t get_children() const
        {
            IParent::children_list_t result;
            return result;
        }
    };
    
    template <typename T>
    bool has_adapter(tinfra::adaptable& a) {
        T* r;
        return a.get_adapter<T>(r);
    }
    
    TEST(adaptable_api)
    {
        Item   item("foo");
        Folder folder("bar");
        
        CHECK( has_adapter<ILabeled>(item) );
        CHECK( !has_adapter<IParent>(item) );
        
        CHECK( has_adapter<ILabeled>(folder) );
        CHECK( has_adapter<IParent>(folder) );
    }
}

// jedit: :tabSize=8:indentSize=4:noTabs=true:mode=c++:
