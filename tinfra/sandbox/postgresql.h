//
// Copyright (c) 2009, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_postgres_h_included
#define tinfra_postgres_h_included

#include <iterator>

#include <tinfra/tinfra.h>

#include <libpq-fe.h>
namespace tinfra { namespace postgres {

/*
    template
*/

typedef std::vector<tinfra::Symbol> symbol_vector;
typedef std::map<tinfra::Symbol, std::string> result_map;

namespace detail {
    void get_symbols(PGresult const* result, symbol_vector& dest)
    {
            
            int n_fields = PQnfields(result);
            dest.reserve(n_fields);
            for(int i  = 0; i < n_fields; ++i ) {
                    char* column_name = PQfname(result,i);
                    Symbol column = Symbol::get(column_name);
                    dest.push_back(column);
            }
    }
}

typedef std::vector<tinfra::Symbol> symbol_vector;
template <typename T>
        void fetch_row(const PGresult* result, int row_number, symbol_vector const& symbols, T& dest)
        {
                int ic = 0;
                for( symbol_vector::const_iterator is = symbols.begin();
                     is != symbols.end();
                     ++is,++ic)
                {
                        if( PQgetisnull(result,row_number, ic) ) continue;
                        char* value = PQgetvalue(result, row_number, ic);
                        tinfra::lexical_set<T>(dest, *is, value);
                }
        }    
    
void fetch_row(const PGresult* result, int row_number, symbol_vector const& symbols, result_map& dest)
{
        int ic = 0;
        for( symbol_vector::const_iterator is = symbols.begin();
             is != symbols.end();
             ++is,++ic)
        {
                if( PQgetisnull(result,row_number, ic) ) continue;
                char* value = PQgetvalue(result, row_number,ic);
                dest[*is] = value;
        }
}    
    
template <typename T>
void fetch_row(const PGresult* result, int row_number, T& dest) {
        symbol_vector symbols;
        detail::get_symbols(result, symbols);
        fetch(result, row_number, symbols, dest);
}
void fetch_row(const PGresult* result, int row_number, result_map& dest)
{
        symbol_vector symbols;
        detail::get_symbols(result, symbols);
        fetch(result, row_number, symbols, dest);
}


template <typename T>
void fetch_all(const PGresult* result, std::vector<T>& dest)
{
        int row_count = PQntuples(result);
        dest.reserve(row_count);
        
        symbol_vector symbols;
        detail::get_symbols(result, symbols);
        std::back_insert_iterator<std::vector<T> > inserter(dest);
        for(int row = 0; row < row_count; ++row,++inserter ) {
                fetch_row(result, row, symbols, *inserter);
                
        }
}


} }

#endif /*tinfra_postgres_h_included */
