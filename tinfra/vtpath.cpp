#include "vtpath.h" // we implement this

#include "tinfra/tstring.h"
#include "tinfra/any.h"
#include "tinfra/trace.h"

#include <vector>
#include <stack>
#include <deque>
#include <iterator>
#include <string>
#include <stdexcept>

namespace tinfra {

//
// vtpath_visit
// 
std::vector<const variant*> vtpath_visit(variant const& v, tstring const& expression)
{
    vtpath_visitor vtpv(const_cast<variant*>(&v), expression);
    
    return std::vector<const variant*>(vtpv.begin(), vtpv.end());
}

std::vector<variant*> vtpath_visit(variant& v, tstring const& expression)
{
    vtpath_visitor vtpv(&v, expression);
    
    return std::vector<variant*>(vtpv.begin(), vtpv.end());
}

//
// vtpath_visitor
// 

enum vpath_token_type {
    ROOT,
    CURRENT_OBJECT,
    CHILD,
    RECURSIVE_CHILD,
    WILDCARD_ALL,
    WILDCARD_OTHER,
    TOKEN
};
struct vtpath_command {
    vpath_token_type type;
    
    variant expr;
    
    vtpath_command(vpath_token_type t): type(t) {}
    vtpath_command(vpath_token_type t, variant expr): type(t), expr(expr) {}
};

//
// vtpath_parse
//
tinfra::module_tracer vtpath_parse_tracer(tinfra::tinfra_tracer, "vtpath.parse");

void vtpath_parse_fail(std::string const& message)
{
    throw std::runtime_error(tsprintf("vtpath: %s", message)); 
}

void vtpath_parse_expect(char v, char expect, const char* message)
{
    if( v != expect ) {
        vtpath_parse_fail(tsprintf("expected '%s', but found '%s': %s", expect, v, message)); 
    }
}


tstring vpath_parse_selector(tstring expr, std::vector<vtpath_command>& result)
    // this function expects string is just after parsing starting [
    // it will consume ending ]
    // only [*] supported for now
{
     while( expr.size() > 0 ) {
        switch(expr[0]) {
            case '*':
                TINFRA_TRACE(vtpath_parse_tracer, "[*] -> WILDCARD_ALL");
                result.push_back(vtpath_command(WILDCARD_ALL));
                vtpath_parse_expect(expr[1],']', "* is supported only as alone token in selector");
                return expr.substr(2);
            default:
                vtpath_parse_fail("only [*] wildcard is supported for now");
                break;
        }
     }
     return expr;
}

void vtpath_parse(tstring expr, std::vector<vtpath_command>& result)
{
    while( expr.size() > 0 ) {
        switch(expr[0]) {
        case '$':
            TINFRA_TRACE(vtpath_parse_tracer, "$ -> ROOT");
            result.push_back(vtpath_command(ROOT));
            expr=expr.substr(1);
            continue;
        case '.':
            if( expr.size() > 1 && expr[1] == '.' ) {
                TINFRA_TRACE(vtpath_parse_tracer, ".. -> RECURSIVE_CHILD");
                result.push_back(vtpath_command(RECURSIVE_CHILD));
                expr=expr.substr(2);
            } else {
                TINFRA_TRACE(vtpath_parse_tracer, ". -> CHILD");
                result.push_back(vtpath_command(CHILD));
                expr=expr.substr(1);
            }
            continue;
        case '*':
            TINFRA_TRACE(vtpath_parse_tracer, "* -> WILDCARD_ALL");
            result.push_back(vtpath_command(WILDCARD_ALL));
            expr=expr.substr(1);
            continue;
        case '[':
            expr = vpath_parse_selector(expr.substr(1), result);
            continue;
        default:
            {
                std::string name;
                while( expr.size() > 0 && (isalnum(expr[0]) || expr[0] == '_'))  {
                        name += expr[0];
                        expr=expr.substr(1);
                }
                TINFRA_TRACE(vtpath_parse_tracer, name << " -> TOKEN(" << name << ")");
                result.push_back(vtpath_command(TOKEN, variant(name)));
                continue;
            }
        }
    }
    
}

tinfra::module_tracer vtpath_exec_tracer(tinfra::tinfra_tracer, "vtpath.exec");

struct vtpath_parse_state {
    variant*                      current;
    
    // iterator in container
    variant::array_type::iterator iarray;
    variant::dict_type::iterator  idict;
    int                           index;
    bool                          matching_finished;
    // iterator in expression-set
    std::vector<vtpath_command>::iterator icommand;
};

struct vtpath_visitor::internal_data {
    variant*   root;
    //std::stirng expression;
    
    std::vector<vtpath_command> commands;
    
    std::stack<vtpath_parse_state> states;
    
    std::deque<variant*> result_queue;
    // helper methods
    vtpath_parse_state& top() {
        return this->states.top();
    }
    
    // process a match
    // node, a matched node
    // command, next command to process
    void match_found(variant* node, std::vector<vtpath_command>::iterator inext_command)
    {
        if( inext_command == commands.end() ) {
            TINFRA_TRACE(vtpath_exec_tracer, "vpath_visit: matched result: " << node << ": " << *node);
            // if we're at end of pattern, i.e no rules
            // that means that we've been matched "directly"
            // so mark CURRENT as a result
            result_queue.push_back(node);
            return;
        } else {
            push_container(node, inext_command);
        }
    }
    void push_container(variant* node, std::vector<vtpath_command>::iterator inext_command)
    {
        vtpath_parse_state new_state;
        new_state.matching_finished = false;
        new_state.current  = node;
        new_state.icommand = inext_command;
        new_state.index = 0;
        
        if( node->is_dict() ) {
            TINFRA_TRACE(vtpath_exec_tracer, "vpath_visit: pushing dict: " << node);
            new_state.idict = node->get_dict().begin();
        } else if( node->is_array() ) {
            TINFRA_TRACE(vtpath_exec_tracer, "vpath_visit: pushing array: " << node);
            new_state.iarray = node->get_array().begin();
        } else {
            // do nothing when
            // we've found leafs not at end of 
            // command chain
            return;
        }
        this->states.push(new_state);
    }
};

vtpath_visitor::vtpath_visitor(variant* v, tstring const& expression):
    self(new internal_data())
{
    self->root = v;
    vtpath_parse(expression, self->commands);
    if( self->commands.size() == 0 ) {
        vtpath_parse_fail("empty predicate");
    }
    if( self->commands[0].type != ROOT ) {
        vtpath_parse_fail("first element shall be root");
    }
    self->match_found(self->root, self->commands.begin()+1);
}
vtpath_visitor::~vtpath_visitor()
{
}
inline std::ostream& operator <<(std::ostream& s, vtpath_command const& cmd)
{
    s << '(';
    switch(cmd.type) {
    case ROOT:            s << "ROOT"; break;
    case CURRENT_OBJECT:  s << "CURRENT_OBJECT"; break;
    case CHILD:           s << "CHILD"; break;
    case RECURSIVE_CHILD: s << "RECURSIVE_CHILD"; break;
    case WILDCARD_ALL:    s << "WILDCARD_ALL"; break;
    case WILDCARD_OTHER:  s << "WILDCARD_OTHER"; break;
    case TOKEN:           s << "TOKEN"; break;
    }
    if( !cmd.expr.is_none() ) {
        s << "," << cmd.expr;
    }
    s << ')';
    return s;
}

bool vtpath_visitor::fetch_next(variant*& r)
{
    while( self->result_queue.empty() ) {
        if( self->states.empty() )
            return false;
        
        vtpath_parse_state& top = self->top();
        if( top.matching_finished ) {
            TINFRA_TRACE(vtpath_exec_tracer, "vpath_visit: matching finished in " << top.current);
            self->states.pop();
            continue;
        }
        bool recursive = false;
        if( top.icommand->type == CHILD ) {
            // nothing
        } else if( top.icommand->type == RECURSIVE_CHILD ) {
            recursive = true;
        } else {
            TINFRA_TRACE(vtpath_exec_tracer, "vpath_visit: state bad " << top.icommand->type << " popping state");
            self->states.pop();
            continue;
        }
        TINFRA_TRACE(vtpath_exec_tracer, "vpath_visit: node: " << (*top.current) << "(" <<top.current << ") " << *(top.icommand) << " index=" << top.index << " recursive=" << (int)recursive);
        
        // we still need at least one command
        TINFRA_ASSERT(top.icommand != self->commands.end()-1);
        vtpath_command const& child_match = *(top.icommand+1);
        TINFRA_TRACE(vtpath_exec_tracer, "vpath_visit: node: next_command= " << *(top.icommand+1));
        
        std::vector<vtpath_command>::iterator inext_command = (top.icommand+2);
        
        if( top.current->is_dict()) {
            variant::dict_type& dict = top.current->get_dict();
            if( child_match.type == WILDCARD_ALL || recursive) {
                // CURRENT.*, or recursive
                // in wildcard we MATCH all
                // in recursive we recurse to all
                if( top.idict != dict.end() ) {
                    variant::dict_type::iterator idict = top.idict;
                    ++top.idict;
                    ++top.index;
                    
                    variant& match = idict->second;
                    TINFRA_TRACE(vtpath_exec_tracer, "vpath_visit: trying: " << top.index-1 << ": " << idict->first << " -> " << match);
                    if(        child_match.type == WILDCARD_ALL ) {
                        // CURRENT.*, matches everything
                        self->match_found(&match, inext_command);
                    } else if( child_match.type == TOKEN ) {
                        // CURRENT.TOKEN
                        if( child_match.expr.is_string() ) {
                            // CURRENT.TOKEN(string)
                            //  -> check if current matches TOKEN in dict
                            TINFRA_ASSERT(child_match.expr.is_string());
                            std::string const& token = child_match.expr.get_string();
                            if( idict->first == token ) {
                                self->match_found(&match, inext_command);
                            }
                        }
                    } else {
                        TINFRA_ASSERT(false);
                    }
                    if( recursive ) {
                        // in recursive must reapply all rules from now on all containers
                        self->push_container(&match, top.icommand);
                    }
                } else {
                    top.matching_finished = true;
                }
            } else if( child_match.type == TOKEN ) {
                // CURRENT.TOKEN, non-recursive
                //  -> search for TOKEN in dict
                std::string const& token = child_match.expr.get_string();

                variant::dict_type::iterator i = dict.find(token);
                top.matching_finished = true;
                if( i != dict.end() ) {
                    variant& match = i->second;
                    self->match_found(&match, inext_command);
                }
            }
        } else if( top.current->is_array() ) {
            variant::array_type& array = top.current->get_array();
            if( child_match.type == WILDCARD_ALL || recursive) {
                // in wildcard we MATCH all
                // in recursive we recurse to all
                if( top.iarray != array.end() ) {
                    variant::array_type::iterator iarray = top.iarray;
                    top.iarray++;
                    top.index++;
                    
                    variant& match = *iarray;
                    TINFRA_TRACE(vtpath_exec_tracer, "vpath_visit: trying: " << top.index-1 << ": " << match);
                    if(        child_match.type == WILDCARD_ALL ) {
                        self->match_found(&match, inext_command);
                    } else if( child_match.type == TOKEN ) {
                        if( child_match.expr.is_integer()) {
                            int current_index = (iarray - array.begin());
                            int index = child_match.expr.get_integer();
                            if( index == current_index ) {
                                self->match_found(&match, inext_command);
                            }
                        }
                    } else {
                        TINFRA_ASSERT(false);
                    }
                    
                    if( recursive ) {
                        // in recursive must reapply all rules from now on all containers
                        self->push_container(&match, top.icommand);
                    }
                } else {
                    top.matching_finished = true;
                }
            } else if( child_match.type == TOKEN ) {
                TINFRA_ASSERT(child_match.expr.is_integer());
                int index = child_match.expr.get_integer();
                top.matching_finished = true;
                if( index >= 0 && index < array.size() ) {
                    variant& match = array[index];
                    std::vector<vtpath_command>::iterator inext_command = (top.icommand+2);

                    self->match_found(&match, inext_command);
                }
                
            }
        } // is array
    } // while result_queue is empty
    r = self->result_queue.front();
    self->result_queue.pop_front();
    return true;
}

} // end namespace tinfra

