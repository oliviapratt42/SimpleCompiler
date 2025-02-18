#ifndef resolution
#define resolution

#include <vector>
#include <string>
#include "parser.h"

extern int syntax_execution; // Declaration

//syntax error management 
int syntax_execution_get();
void syntax_execution_set(int status);

//semantic handling 
void task_execution(PROGRAM* program);
void task_1(PROGRAM* program);
    //helper programs
    bool code_1(vector<POLY_DECL>* poly_section);
    bool code_2(vector<POLY_DECL>* poly_section);
        vector<int> process_term_list_univariate(TERM_LIST* term_list, vector<int> invalid_line);
        vector<int> process_term_list_multivariate(TERM_LIST* term_list, vector<int> invalid_line, const vector<string>& poly_vars);
    bool code_3(PROGRAM* program);
    bool code_4(PROGRAM* program);
//data structure construction
int grow_varmap(string lexeme);
int get_varmap(string lexeme);
int get_valuelist(int index, int value);
int set_valuelist(int index, int value);

void printTasks(PROGRAM* program);
void printPOLYDECL(PROGRAM* program);
    void print_term_list(TERM_LIST* term_list);
    void print_poly_decl(const POLY_DECL& poly);
void printINPUTS(PROGRAM* program);
void printEXECUTE(PROGRAM* program);

#endif