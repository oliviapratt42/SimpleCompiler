#ifndef resolution
#define resolution

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "parser.h"
#include "lexer.h"
#include "inputbuf.h"

extern int syntax_execution; // Declaration
extern int variables[1000];//index of variables = 1 if variable 

//syntax error management 
int syntax_execution_get();
void syntax_execution_set(int status);

//semantic handling 
void task_execution(PROGRAM* program);

void task_5(PROGRAM* program);
int count_poly(POLY_DECL* poly_decl);
int count_term(TERM* term);
int count_monomial(MONOMIAL* monom);
int count_primary(PRIMARY* primary);

void task_4(PROGRAM* program);
void check_var_use(ARGUMENT_LIST* arguement_list);
void update_use(string lexeme, bool status);

void task_3(PROGRAM* program);
void validate_decl(ARGUMENT_LIST* arg_list);

void task_2(PROGRAM* program);
    void process_inputs(vector<int>* inputs);
    int resolve_polynomial(POLY_EVAL* poly_eval, POLY_DECL* poly_decl);
    int resolve_argument(ARGUMENT* argument);
    int resolve_term(TERM* term, const vector<int>& arg_vals);
    int resolve_monomial(MONOMIAL* monom, const vector<int>& arg_vals);
    int resolve_primary(PRIMARY* primary, const vector<int>& arg_vals);

bool task_1(PROGRAM* program);
    //helper programs
    bool code_1(vector<POLY_DECL>* poly_section);
    bool code_2(vector<POLY_DECL>* poly_section);
        vector<int> process_term_list(TERM_LIST* term_list, vector<int> invalid_line, const vector<string>& poly_vars);
    bool code_3(PROGRAM* program);
        void check_poly_eval(POLY_EVAL* poly_eval, unordered_set<string> polyNames, vector<int>& error_lines);
        void check_arg_list(ARGUMENT_LIST* argument_list,unordered_set<string> polyNames, vector<int>& error_lines);
    bool code_4(PROGRAM* program);
        vector<int> compare_argument_count(vector<POLY_DECL>* poly_section, POLY_DECL* poly_decl, POLY_EVAL* poly_eval, vector<int> error_lines);
        POLY_DECL* find_poly_decl(vector<POLY_DECL>* poly_section, const string& name);

        //data structure construction
int grow_varmap(string lexeme);
int get_varmap(string lexeme);

void add_to_warning(int line);

void printTasks(PROGRAM* program);
void printPOLYDECL(PROGRAM* program);
    void print_term_list(TERM_LIST* term_list);
    void print_poly_decl(const POLY_DECL& poly);
void printINPUTS(PROGRAM* program);
void printEXECUTE(PROGRAM* program);

#endif