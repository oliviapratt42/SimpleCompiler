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
    //collect int vector
    parse_tasks_section();
    //collect poly_decl list
    parse_poly_section();
    //collect statement list
    parse_execute_section();
    //collect int vector
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
    parse_poly_dec_list();
    return;
}
void Parser::parse_poly_dec_list(){
    //poly_decl or poly_decl poly_dec_list
    parse_poly_decl();
    Token t = lexer.peek(1);
    Token s = lexer.peek(2);
    //question: then we have found a polyheader and need to parse decl unti end of list
    while (t.token_type == ID && s.token_type == LPAREN){
        parse_poly_decl();
    }
    return;
}
void Parser::parse_poly_decl(){
    //poly_header EQUAL poly_body SEMICOLON
    parse_poly_header(); //returns a list 
    expect(EQUAL);
    parse_poly_body();
    expect(SEMICOLON);
    return;
}
void Parser::parse_poly_header(){
    //poly_name
    parse_poly_name();
    Token t = lexer.peek(1);
    if (t.token_type == LPAREN){
        expect(LPAREN);
        parse_id_list();
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
        parse_id_list();
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
    parse_term_list();
    return;
}
void Parser::parse_term_list(){
    //term
    //term add_operator term_list
    parse_term();
    Token t = lexer.peek(1);
    if (t.token_type == PLUS || t.token_type == MINUS){
        parse_add_operator();
        parse_term_list();
    }
}
void Parser::parse_term(){
    //monomial_list
    //coefficient monomial_list
    //coefficient -> NUM
    Token t = lexer.peek(1);
    //monom_list ->monom->primary_>ID or Lparen
    if (t.token_type == ID || t.token_type == LPAREN){
        //create new monomial list
        parse_monomial_list();
    }
    else if (t.token_type == NUM){
        parse_coefficient();
        //if monomial list follows
        if (t.token_type == ID || t.token_type == LPAREN){
                    //create new monomial list

            parse_monomial_list();
        }
    }
    //what can come after a term: add_operator, term, semicolon, 
    t = lexer.peek(1);
    if (t.token_type == PLUS || t.token_type == MINUS || t.token_type == ID || t.token_type == NUM || t.token_type == LPAREN || t.token_type == SEMICOLON){
        return;
    }
    else syntax_error();
}
void Parser::parse_monomial_list(){
    //monomial or monomial list
    Token t = lexer.peek(1);
    //monom_list ->monom->primary_>ID or Lparen
    while (t.token_type == ID || t.token_type == LPAREN){
        parse_monomial();
        //adjust LL pointers
    }
    return;
}
void Parser::parse_monomial(){
    //primary or primary exponent
    //allocate memory to monomial struct
    parse_primary();
    Token t = lexer.peek(1);
    if (t.token_type == POWER){
        parse_exponent();
    }
    return;
}
void Parser::parse_primary(){
    //ID or LParent term_list RParent
    Token t = lexer.peek(1);
    if (t.token_type == ID){
        Token k = expect(ID);
        return;
    }
    else if(t.token_type == LPAREN){
        Token k = expect(LPAREN);
        parse_term_list();
        Token j = expect(RPAREN);
        return;
    }
    syntax_error();
}
int Parser::parse_exponent(){
    //POWER NUM
    Token t = expect(POWER);
    Token k = expect(NUM);
    int value = 0; //value = k.lexeme(int)
    return value;
}
void Parser::parse_add_operator(){
    //PLUS or MINUS
    Token t = lexer.peek(1);
    if (t.token_type == PLUS) {Token k = expect(PLUS);}
    else if (t.token_type == PLUS) {Token k = expect(PLUS);}
    //can follow by termlist or term ID LPAREN or NUM
    t = lexer.peek(1);
    if (t.token_type == ID || t.token_type == LPAREN || t.token_type == NUM){
        return;
    }
    syntax_error();
}
int Parser::parse_coefficient(){
    //NUM
    Token t = expect(NUM);
    int value = 0;
    //FIXME int value = (int)t.lexeme;
    return value;
}
void Parser::parse_execute_section(){
    //EXECUTE statementnt_list
    Token t = expect(EXECUTE);
    //need a new statement_list struct init and assign to execute_section of program
    parse_statement_list();
    return;
}
void Parser::parse_statement_list(){
    //statement opt (statement_list)
    //input, output, assign
    Token t = lexer.peek(1);
    while (t.token_type == ID || t.token_type == OUTPUT || t.token_type == INPUT){
        parse_statement();
        //statement list next pointer 
    }
    return;
}
void Parser::parse_statement(){
    //input, output, assign
    //make new statement malloc and pass to pasre x statement
    Token t = lexer.peek(1);
    if (t.token_type == INPUT){
        parse_input_statement();
        //return statement struct
        return;
    }
    else if (t.token_type == OUTPUT){
        parse_output_statement();
        //return statement struct
        return;
    }
    else if (t.token_type == ID){
        parse_assign_statement();
        //return statement struct
        return;
    }
    //error handling
    syntax_error();
}
void Parser::parse_input_statement(){
    //input id semicolon
    //fill values of statement struct param
    Token t = expect(INPUT);
    Token k = expect(ID);
    Token j = expect(SEMICOLON);
    //return statement struct
    return;
}
void Parser::parse_output_statement(){
    //output id semicolon
    //fill values of statement struct param
    Token t = expect(OUTPUT);
    Token k = expect(ID);
    Token j = expect(SEMICOLON);
    //return statement struct
    return;
}
void Parser::parse_assign_statement(){
    //ID EQUAL poly_evaluation SEMICOLON
    //fill values of statement struct param
    Token t = expect(ID);
    Token k = expect(EQUAL);
    parse_poly_evaluation();
    Token j = expect(SEMICOLON);
    //return statement struct
    return;
}
void Parser::parse_poly_evaluation(){
    //poly_name LPAREN argument_list RPAREN
    parse_poly_name();
    Token t = expect(LPAREN);
    parse_argument_list();
    t = lexer.peek(1);
    //comma or semicolon
    if (t.token_type == COMMA || t.token_type == SEMICOLON){
        return;
    }
    syntax_error();
}
void Parser::parse_argument_list(){
    /*  argument
        argument COMMA argument_list
    */
    //TODO
    parse_argument();
    Token t = lexer.peek(1);
    if (t.token_type == COMMA){
        Token k = expect(COMMA);
        parse_argument_list();
    }
        //or end of poly eval
    else if (t.token_type == RPAREN){
        return;
    }
    syntax_error();
}
void Parser::parse_argument(){
    /*  ID
        NUM
        poly_evaluation -> poly_name -> ID
    */
   //TODO
    Token t = lexer.peek(1);
    if (t.token_type == ID){
        t = expect(ID);
        return;
    }
    else if (t.token_type == NUM){
        t = expect(NUM);
        return;
    }
    else if (t.token_type == ID){
        parse_poly_evaluation();
        return;
    }
    syntax_error();
}
void Parser::parse_inputs_section(){
    //INPUTS num_list
    //return int vector inputs_section
    Token t = expect(INPUTS);
    parse_num_list();
    return;
}

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
        /*
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
        */
int main()
{
    // note: the parser class has a lexer object instantiated in it. You should not be declaring
    // a separate lexer object. You can access the lexer object in the parser functions as shown in the
    // example method Parser::ConsumeAllInput
    // If you declare another lexer object, lexical analysis will not work correctly
    Parser parser;

    parser.parse_program();
    Token E_O_F = parser.expect(END_OF_FILE);
}
