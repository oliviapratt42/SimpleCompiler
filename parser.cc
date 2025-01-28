/*
 * Copyright (C) Rida Bazzi, 2020
 *
 * Do not share this file with anyone
 *
 * Do not post this file or derivatives of
 * of this file online
 *
 */
#include <iostream>
#include <cstdlib>
#include "parser.h"

using namespace std;

void Parser::syntax_error()
{
    cout << "SYNTAX ERROR !!!!!&%!!\n";
    exit(1);
}


void Parser::parse_program(){
    //tasks_section poly_section execute_section inputs_section
    parse_tasks_section();
    parse_poly_section();
    parse_execute_section();
    parse_inputs_section();
    return;
    }
void Parser::parse_tasks_section(){
    //TASKS num_list
    expect(TASKS);
    parse_num_list();
    return;
}
void Parser::parse_num_list(){
    //NUM
    //NUM num_list
    Token t = expect(NUM);
    Token s = lexer.peek(1);
    if (s.token_type == NUM){ //question: argument vs numlist
        parse_num_list();
    }
    return;
    //question: should I check for other options before syntax error?
}
void Parser::parse_poly_section(){
    //POLY poly_decl_list
    Token t = expect(POLY);
    parse_poly_dec_list;
    return;
}
void Parser::parse_poly_dec_list(){
    //poly_decl or poly_decl poly_dec_list
    parse_poly_decl;
    Token t = lexer.peek(1);
    Token s = lexer.peek(2);
    //question: then we have found a polyheader and need to parse decl unti end of list
    while (t.token_type == ID && s.token_type == LPAREN){
        parse_poly_decl;
    }
    return;
}
void Parser::parse_poly_decl(){
    //poly_header EQUAL poly_body SEMICOLON
    parse_poly_header;
    expect(EQUAL);
    parse_poly_body;
    expect(SEMICOLON);
    return;
}
void Parser::parse_poly_header(){
    //poly_name
    parse_poly_name();
    Token t = lexer.peek(1);
    if (t.token_type == LPAREN){
        expect(LPAREN);
        parse_id_list;
        expect(RPAREN);
    }
    return;
}
void Parser::parse_id_list(){
    //ID
    //ID COMMA id_list
    expect(ID);
    Token t = lexer.peek(1);
    if (t.token_type == COMMA){
        expect(COMMA);
        parse_id_list;
    }
    return;
}
void Parser::parse_poly_name(){
    //ID
    expect(ID);
    return;
}
void Parser::parse_poly_body(){
    //term_list
    parse_term_list;
    return;
}
void Parser::parse_term_list(){
    //term
    //term add_operator term_list
    parse_term;
    Token t = lexer.peek(1);
    if (t.token_type == PLUS || t.token_type == MINUS){
        parse_add_operator;
        parse_term_list;
    }
}
void Parser::parse_term();
    void parse_monomial_list();
    void parse_monomial();
    void parse_primary();
    void parse_exponent();
    void parse_add_operator();
    void parse_coefficient();
    void parse_execute_section();
    void parse_statement_list();
    void parse_statement();
    void parse_input_statement();
    void parse_output_statement();
    void parse_assign_statmement();
    void parse_poly_evaluation();
void parse_argument_list();
void parse_argument();
void parse_inputs_section();

// this function gets a token and checks if it is
// of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated
// this function is particularly useful to match
// terminals in a right hand side of a rule.
// Written by Mohsen Zohrevandi
Token Parser::expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}

// Parsing

// This function is simply to illustrate the GetToken() function
// you will not need it for your project and you can delete it
// the function also illustrates the use of peek()
void Parser::parse_input()
{
    Token token;
    int i = 1;

    token = lexer.peek(i);
    token.Print();
    while (token.token_type != END_OF_FILE)
    {
        i = i + 1;
        token = lexer.peek(i);
        token.Print();
    }

    token = lexer.GetToken();
    token.Print();
    while (token.token_type != END_OF_FILE)
    {
        token = lexer.GetToken();
        token.Print();
    }
}

int main()
{
    // note: the parser class has a lexer object instantiated in it. You should not be declaring
    // a separate lexer object. You can access the lexer object in the parser functions as shown in the
    // example method Parser::ConsumeAllInput
    // If you declare another lexer object, lexical analysis will not work correctly
    Parser parser;

    parser.parse_input();
}
