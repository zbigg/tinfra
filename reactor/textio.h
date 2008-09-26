//
// Copyright (C) Zbigniew Zagorski <z.zagorski@gmail.com>,
// licensed to the public under the terms of the GNU GPL (>= 2)
// see the file COPYING for details
// I.e., do what you like, but keep copyright and there's NO WARRANTY.
//

#ifndef __theg_textio_h__
#define __theg_textio_h__

#include "converter.h"
#include "tinfra/tinfra.h"
#include "tinfra/tinfra_lex.h"

/// TextIO WireFormat
struct TextWire {
    typedef std::string RequestType;
    typedef std::string ResponseType;
};

struct TextRequestWire {
    typedef std::string RequestType;
    typedef int         ResponseType;
};

namespace TextIOImpl {
    class ContextBasic {
        std::vector<tinfra::symbol> context_;
        bool using_context_;
    protected:    
        ContextBasic(); // default value is implementation specific
        ContextBasic(bool using_context): using_context_(using_context) {}
            
        int  check_context(tinfra::symbol s, std::string const& input);
        void realize_context(tinfra::symbol s, std::ostream& output);
        void push_context(tinfra::symbol s);
        void pop_context();
    };
    
    class Reader: public ContextBasic {
        std::istream& src_;
    public:
        Reader(std::istream& src): src_(src) {}
        
        template <typename T>
        void list_container(T& container, tinfra::symbol const& symbol)
        {
            push_context(symbol);
            int size;
            const tinfra::symbol size_sym = "size";
            const tinfra::symbol item_sym = "item";
            tinfra::TypeTraits<int>::mutate(size, size_sym, *this);
            for( int i = 0; i < size; ++i ) {
                typename T::value_type tmp;
                tinfra::TypeTraits<typename T::value_type>::mutate(tmp, item_sym, *this);
                container.push_back(tmp);
            }
            pop_context();
        }
        
        template <typename T>
        void operator() (tinfra::symbol const& sym, T& value)
        {
            std::string tmp;
            std::getline(src_, tmp);            
            int ofs = check_context(sym, tmp);
            tinfra::from_string(tmp.c_str() + ofs, value);
        }
    };
    
    class Writer: public ContextBasic  {
        std::ostream& dest_;
    public:
        Writer(std::ostream& dest): dest_(dest) {}
        
        template <typename T>
        void list_container(T const& container, tinfra::symbol const& symbol)
        {
            push_context(symbol);
            const tinfra::symbol size_sym = "size";
            const tinfra::symbol item_sym = "item";
            int size = container.size();
            tinfra::TypeTraits<int>::process(size, size_sym, *this);
            for( typename T::const_iterator i = container.begin(); i != container.end(); ++i ) {
                tinfra::TypeTraits<typename T::value_type>::process(*i, item_sym, *this);
            }
            pop_context();
        }
        
        template <typename T>
        void operator() (tinfra::symbol const& sym, T const& value)
        {
            realize_context(sym, dest_);
            tinfra::to_string(value, dest_);
            dest_ << std::endl;
        }
    };
} // end namespace TextIOImpl

template<>
struct Converter<TextIO> {
    template <typename TO>
    static void convert(std::string const& source, TO& dest)
    {
        std::istringstream input(source);
        TextIOImpl::Reader reader(input);
        tinfra::tt_mutate(dest, reader);
    }
    template <typename FROM>
    static void convert(FROM& source, std::string& dest)
    {
        std::ostringstream output;
        TextIOImpl::Writer writer(output);
        tinfra::tt_process(source, writer);
        dest = output.str();
    }
};

#endif
