/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <vector>
#include <string>
#include <unordered_map>
#include "lexer.h"
#include "inputbuf.h"

using namespace std;

/* Enums */

// Type of arguments
enum ATYPE {
  NUM_TYPE = 0,
  ID_TYPE = 1,
  POLYEVAL_TYPE = 2
};

// Operators for addition/subtraction
enum ADD_OPERATOR {
  NONE = 0,
  PLUS_SIGN = 1,
  MINUS_SIGN = -1
};

// Type of primary elements (Identifier or Term List)
enum PTYPE {
  IDENFIER = 0,
  RTERML = 1
};

// Types of statements (Input, Output, Assign)
enum STYPE {
  INPUT_STMT = 0,
  OUTPUT_STMT = 1,
  ASSIGN_STATEMENT = 2
};

/* Forward Declarations */

// These are declared early because they will be referenced in the structures
struct POLY_EVAL;
struct TERM;
struct MONOMIAL_LIST;
struct STATEMENT_LIST;
struct ARGUMENT_LIST;

// Structure Definitions

// Argument structure for polynomial evaluation
struct ARGUMENT {
  ATYPE type; 
  int value; 
  int index; // of ID in id list 
  POLY_EVAL* poly_eval;
  ARGUMENT* next;
};

// Monomial (a term with an exponent and associated primary)
struct MONOMIAL {
  int exponent;
  struct PRIMARY* primary; 
  MONOMIAL* next; 
};

// List of monomials (could be part of a term)
struct MONOMIAL_LIST {
  MONOMIAL* head; 
};

// Primary term (can be an identifier or a term list)
struct PRIMARY {
  int line_no; // for semantic checking
  PTYPE type; // IDENFIER, TERM_LIST
  int var_index; // index of the primary 
  TERM_LIST* term_list; // points to list of terms
};

// Polynomial declaration structure
struct POLY_DECL {
  string name;
  vector<pair<string, int>>* poly_parameters;
  TERM_LIST* body; // body of the polynomial
  int line_no; // line number in the polynomial declaration section
};
struct TERM_LIST{
  TERM* head;
};
// Term structure for polynomial (may contain a list of monomials and an operator)
struct TERM {
  int coefficient;
  MONOMIAL_LIST* monomial_list;
  ADD_OPERATOR addop; // PLUS (1), MINUS(-1), NONE (0)
  TERM* next; // for linking terms
};

// Statement structure (input, output, or assignment)
struct STATEMENT {
  STYPE statement_type; // input, output, or assign
  int var; // for input/output: index of location
  int LHS; // index of variable on the LHS of the equation
  POLY_EVAL* poly_evaluation_t; // optional for assignment
  STATEMENT* next; // next statement in the list
};

// Polynomial evaluation structure
struct POLY_EVAL {
  string name; // of the polynomial in the table
  ARGUMENT_LIST* argument_list; // list of arguments
};

// List of arguments for polynomial evaluation
struct ARGUMENT_LIST {
  ARGUMENT* head; // head of the argument list
};

// Main program structure containing tasks, polynomial declarations, statements, and input sections
struct PROGRAM {
  vector<int>* tasks; // list of task numbers
  vector<POLY_DECL>* poly_section; // list of polynomial declarations
  STATEMENT_LIST* execute_section; // linked list of statements to execute
  vector<int>* inputs_section; // input list
};

// Linked list of statements (acts as a container for statements)
struct STATEMENT_LIST {
  STATEMENT* statement; // head of the statement list
};

class Parser {
  public:
    void parse_input();
    Token expect(TokenType expected_type);
    PROGRAM* parse_program(PROGRAM* program);
    LexicalAnalyzer lexer;

  private:
    PROGRAM* allocate_program();
    void syntax_error();
    void parse_tasks_section(vector<int>* tasks);
    void parse_num_list(vector<int>* nums); 
    void parse_poly_section(vector<POLY_DECL>* poly_declarations);
    void parse_poly_dec_list(vector<POLY_DECL>* poly_declarations);
    void parse_poly_decl(vector<POLY_DECL>* poly_declarations);
    void parse_poly_header(POLY_DECL* poly_decl);
    void parse_id_list(vector<pair<string, int>>* poly_parameters);
    string parse_poly_name();
    void parse_poly_body(POLY_DECL* poly_decl);
    void parse_term_list(TERM_LIST* poly_decl);
    void parse_term(TERM* term);
    void parse_monomial_list(MONOMIAL_LIST* monomial_list);
    void parse_monomial(MONOMIAL* monomial);
    void parse_primary(PRIMARY* primary);
    int parse_exponent();
    ADD_OPERATOR parse_add_operator();
    int parse_coefficient();
    void parse_execute_section(STATEMENT_LIST* statement_list); //create new statement list
    void parse_statement_list(STATEMENT_LIST* statment_list); 
    void parse_statement(STATEMENT* statement);
    void parse_input_statement(STATEMENT* statement);
    void parse_output_statement(STATEMENT* statement);
    void parse_assign_statement(STATEMENT* statement);
    void parse_poly_evaluation(POLY_EVAL* poly_eval);
    void parse_argument_list(ARGUMENT_LIST* argument_list);
    void parse_argument(ARGUMENT* argument);
    void parse_inputs_section(vector<int>* inputs_section);
    };

#endif

