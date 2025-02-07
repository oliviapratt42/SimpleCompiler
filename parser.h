/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <vector>
#include <string>
#include "lexer.h"
#include "inputbuf.h"

using namespace std;

/*
OBJECT DEFINITIONS
*/
struct PROGRAM{
  vector<int> tasks; //num list
  //instead use a vector table
  POLY_DECL* poly_section;
  STATEMENT_LIST* execute_section; //statement list
  vector<int> inputs_section; //num list
};
//implemented as a LL
struct STATEMENT_LIST{
  STATEMENT* statement; 
  STATEMENT* next;
};
struct STATEMENT{
  STYPE statement_type; //input, output or assign
  string ID; 
  int poly_evaluation = 0; //optional for assign
};
struct POLY_DECL{
    char* name;
    VAR_MAP* arg_map;
    TERM* body;
};
//behaves as a linked list 
struct TERM{
    int coefficient;
    MONOMIAL* monomial;
    ADD_OPERATOR addop; //PLUS, MINUS, NONE
    TERM* next;
};
struct MONOMIAL {
    int exponent;
    PRIMARY* primary; 
    MONOMIAL* next; 
};
struct PRIMARY {
    int line_no; //for semantic checking
    PTYPE type; //IDENTIFIER, TERM_LIST
    Token id; 
    TERM* term_list;
};
//!!!TODO implement hash map!!!
struct VAR_MAP{
    string var;
    int value;
};
enum ADD_OPERATOR{
    NONE = 0,
    PLUS_SIGN = 1,
    MINUS_SIGN = -1
};
enum PTYPE{
    IDENFIER = 0,
    RTERML = 1
};
enum STYPE{
  INPUT_STMT = 0,
  OUTPUT_STMT = 1,
  ASSIGN_STATEMENT = 2
};

class Parser {
  public:
    void parse_input();
    Token expect(TokenType expected_type);
    void parse_program();

  private:
    LexicalAnalyzer lexer;
    void syntax_error();
    void parse_tasks_section();
    void parse_num_list(); 
    void parse_poly_section();
    void parse_poly_dec_list();
    void parse_poly_decl();
    void parse_poly_header();
    void parse_id_list();
    void parse_poly_name();
    void parse_poly_body();
    void parse_term_list();
    void parse_term();
    void parse_monomial_list();
    void parse_monomial();
    void parse_primary();
    void parse_exponent();
    void parse_add_operator();
    void parse_coefficient();
    void parse_execute_section(); //create new statement list
    void parse_statement_list(); 
    void parse_statement();
    void parse_input_statement();
    void parse_output_statement();
    void parse_assign_statement();
    void parse_poly_evaluation();
    void parse_argument_list();
    void parse_argument();
    void parse_inputs_section();
    };

#endif

