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
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include "parser.h"
#include "resolution.h"
#include "lexer.h"
#include "inputbuf.h"

using namespace std;


void Parser::syntax_error()
{
    cerr << "SYNTAX ERROR !!!!!&%!!\n";
    syntax_execution_set(1); //send saying execution happened
    exit(1);
}

PROGRAM* Parser::parse_program(PROGRAM* program){
    //tasks_section poly_section execute_section inputs_section

    parse_tasks_section(program->tasks);
    //printTasks(program);

    parse_poly_section(program->poly_section);
    printPOLYDECL(program);

    parse_execute_section(program->execute_section);
   // printEXECUTE(program);
    parse_inputs_section(program->inputs_section);
    //printINPUTS(program);
    return program;
    }
void Parser::parse_tasks_section(vector<int>* tasks){
    //TASKS num_list
    Token t = expect(TASKS);
    parse_num_list(tasks);
    return;
}
void Parser::parse_num_list(vector<int>* nums){
    //NUM
    //NUM num_list

    Token t = expect(NUM);
    nums->push_back(stoi(t.lexeme));//grow num list

    Token s = lexer.peek(1);
    if (s.token_type == NUM){ 
        parse_num_list(nums); //recursive
    }
    return;
}
void Parser::parse_poly_section(vector<POLY_DECL>* poly_declarations){
    //POLY poly_decl_list
    // Check if peek(1) returned a valid token
    Token k = lexer.peek(1);
    if (k.token_type != POLY) {  // Assuming INVALID is a defined token type for error checking
        cerr << "Unexpected Token in poly section: " << k.token_type << endl;
        syntax_error();
    } 
    
    k = expect(POLY);
    parse_poly_decl_list(poly_declarations);
    return;
}
void Parser::parse_poly_decl_list(vector<POLY_DECL>* poly_declarations){
    //poly_decl or poly_decl poly_dec_list
    parse_poly_decl(poly_declarations);
    Token t = lexer.peek(1);
    if (t.token_type == ID){
        parse_poly_decl_list(poly_declarations); //parse subsequent decl
    }
    else if (t.token_type == EXECUTE){
        return;
    }
    else {
        cerr << "Unexpected token in POLY_DECL_LIST: " << t.token_type << endl;
        syntax_error();
    }

    return;
}
void Parser::parse_poly_decl(vector<POLY_DECL>* poly_declarations){
    //poly_header EQUAL poly_body SEMICOLON
    POLY_DECL* poly_decl = new POLY_DECL();
    if (!poly_decl) {
        cerr << "Memory allocation failed for POLY_DECL" << endl;
        exit(1);
    }
    parse_poly_header(poly_decl); 
    Token k = expect(EQUAL);
    parse_poly_body(poly_decl);
    Token s = expect(SEMICOLON);

    poly_declarations->push_back(*poly_decl); //adding to table
    return;
}

//debugged
void Parser::parse_poly_header(POLY_DECL* poly_decl){
    //poly_name
    Token name = parse_poly_name();
    poly_decl->name = name.lexeme;
    poly_decl->line_no = name.line_no;
    Token t = lexer.peek(1);
    if (t.token_type == LPAREN){
        t = expect(LPAREN);
        int order = 0;
        poly_decl->poly_parameters = new vector<pair<string, int> >();
        parse_id_list(poly_decl->poly_parameters, order); 
        t = expect(RPAREN);
    } 
    else if (t.token_type == EQUAL){
        return;
    }
    else{
        cerr << "Unexpected token at poly header " << t.token_type << " \n";
        syntax_error();
    }
    return;
}

void Parser::parse_id_list(vector<pair<string, int> >* poly_parameters, int order){
    //ID
    //ID COMMA id_list
    Token t = expect(ID);
    poly_parameters->push_back(make_pair(t.lexeme, order)); 
    int success = grow_varmap(t.lexeme);
    if (success < 0){
        cerr << "Error in growing var map with " << t.lexeme << " \n";
    }

    Token k = lexer.peek(1);
    if (k.token_type == COMMA){
        Token s = expect(COMMA);
        order++; //there is another one
        s = lexer.peek(1);
        if (s.token_type == ID){
            parse_id_list(poly_parameters, order);
        }
    }
    else if (k.token_type == RPAREN){
        return;
    }
    else {
        cerr << "Unexpected token ahead when parsing id list: " << t.token_type << "\n";
        syntax_error();
        return;
    }
    return;
}
Token Parser::parse_poly_name(){
    //ID
    Token t = expect(ID);
    return t;
}
void Parser::parse_poly_body(POLY_DECL* poly_decl){
    //term_list
    TERM_LIST* term_list = new TERM_LIST();
    poly_decl->body = term_list;
    parse_term_list(term_list);
    return;
}
void Parser::parse_term_list(TERM_LIST* term_list){
    //term
    //term add_operator term_list
    TERM* term = new TERM();
    parse_term(term);
    //construct LL MAYBE A PROBLEM
    if (term_list->head == nullptr){
        term_list->head = term;
    } else {
        TERM* curr = term_list->head;
        while (curr->next != nullptr){
            curr = curr->next;
        }
        curr->next = term;
    }
    term->next = nullptr;

    Token t = lexer.peek(1);
    if (t.token_type == PLUS || t.token_type == MINUS){
        term->addop = parse_add_operator();
        parse_term_list(term_list);
    }
    else if (t.token_type == SEMICOLON|| t.token_type == RPAREN){
        return;
    }
    else {
        cerr << "Unexpected token ahead when parsing term list: " << t.token_type << "\n";
        syntax_error();
        return;
    }
    return;
}
void Parser::parse_term(TERM* term){
    //monomial_list
    //coefficient monomial_list
    //coefficient -> NUM
    Token t = lexer.peek(1);
    //monom_list ->monom->primary_>ID or Lparen
    if (t.token_type == ID || t.token_type == LPAREN){
        //create new monomial list
        MONOMIAL_LIST* monomial_list = new MONOMIAL_LIST();
        monomial_list->head = nullptr;
        term->monomial_list = monomial_list;
        parse_monomial_list(monomial_list);
    }
    else if (t.token_type == NUM){
        term->coefficient = parse_coefficient();
        //if monomial list follows
        t = lexer.peek(1);
        if (t.token_type == ID || t.token_type == LPAREN){
            //create new monomial list
            MONOMIAL_LIST* monomial_list = new MONOMIAL_LIST();
            monomial_list->head = nullptr;
            term->monomial_list = monomial_list;
            parse_monomial_list(monomial_list);
        }
    }
    //what can come after a term: add_operator, term, semicolon, 
    t = lexer.peek(1);
    if (t.token_type == PLUS || t.token_type == RPAREN || t.token_type == MINUS || t.token_type == ID || t.token_type == NUM || t.token_type == LPAREN || t.token_type == SEMICOLON){
        return;
    }
    else {
        cerr << "Unexpected token ahead when parsing term: " << t.token_type << "\n";
        syntax_error();
        return;
    }
}
void Parser::parse_monomial_list(MONOMIAL_LIST* monomial_list){
    //monomial or monomial list
    Token t = lexer.peek(1);
    //monom_list ->monom->primary_>ID or Lparen
    if (t.token_type == ID || t.token_type == LPAREN){
        MONOMIAL* new_monomial = new MONOMIAL();
        parse_monomial(new_monomial);
        //adjust LL pointers
        MONOMIAL* curr = monomial_list->head;
        if (monomial_list->head == nullptr){
            monomial_list->head = new_monomial;
        }else{
            while (curr->next != nullptr){
                curr = curr->next;
            }
            curr->next = new_monomial;
        }
        new_monomial->next = nullptr;
        parse_monomial_list(monomial_list);
    }
    else if (t.token_type == PLUS || t.token_type == MINUS || t.token_type == NUM|| t.token_type == SEMICOLON || t.token_type == RPAREN){
        return;
    }
    else {
        cerr << "Unexpected token ahead in monom list: " << t.token_type << " line no: " << t.line_no<< "\n";
        syntax_error();
    }
    return;
}
void Parser::parse_monomial(MONOMIAL* monomial){
    //primary or primary exponent
    //allocate memory to monomial struct
    PRIMARY* new_primary = new PRIMARY();
    monomial->primary = new_primary;
    parse_primary(new_primary);
    Token t = lexer.peek(1);
    if (t.token_type == POWER){
        int exponent = parse_exponent();
        monomial->exponent = exponent;
    }
    else{
        monomial->exponent = 1;
    }
    return;
}
void Parser::parse_primary(PRIMARY* primary){
    //ID or LParent term_list RParent
    Token t = lexer.peek(1);
    if (t.token_type == ID){
        Token k = expect(ID);
        primary->type = IDENFIER;
        primary->line_no = t.line_no;
        int success = grow_varmap("x");
        if (success < 0){
            cerr << "Parse primary grow varmap unsuccessful\n";
        }
        return;
    }
    else if(t.token_type == LPAREN){
        Token k = expect(LPAREN);
        TERM_LIST* new_term_list = new TERM_LIST();
        new_term_list->head = nullptr;
        parse_term_list(new_term_list);
        Token j = expect(RPAREN);
        primary->type = RTERML;
        return;
    }
    else {
        cerr << "Unexpected token ahead when parsing primary: " << t.token_type << "\n";
        syntax_error();
        return;
    }
}
int Parser::parse_exponent(){
    //POWER NUM
    Token t = expect(POWER);
    Token k = expect(NUM);
    int value = stoi(k.lexeme); 
    return value;
}
ADD_OPERATOR Parser::parse_add_operator(){
    //PLUS or MINUS
    Token t = lexer.peek(1);
    if (t.token_type == PLUS) {
        Token k = expect(PLUS);
        return PLUS_SIGN;
    }
    else if (t.token_type == MINUS) {
        Token k = expect(MINUS);
        return MINUS_SIGN;
    }
    //can follow by termlist or term ID LPAREN or NUM
    t = lexer.peek(1);
    if (t.token_type == ID || t.token_type == LPAREN || t.token_type == NUM){
        return NONE;
    }
    else {
        cerr << "Unexpected token ahead when parsing add operator: " << t.token_type << "\n";
        syntax_error();
    }    
    return NONE;
}
int Parser::parse_coefficient(){
    //NUM
    Token t = expect(NUM);
    int value = stoi(t.lexeme);
    //FIXME int value = (int)t.lexeme;
    return value;
}
void Parser::parse_execute_section(STATEMENT_LIST* statement_list){
    //EXECUTE statementnt_list
    Token t = expect(EXECUTE);
    if (t.token_type == EXECUTE){
        //need a new statement_list struct init and assign to execute_section of program
        parse_statement_list(statement_list);
        return;
    } else{
        cerr << "Unexpected token ahead in execute section: " << t.token_type << "\n";
        syntax_error();
        return;
    }
    return;
}
void Parser::parse_statement_list(STATEMENT_LIST* statement_list){
    //statement opt (statement_list)
    //input, output, assign
   
    Token t = lexer.peek(1);

    if(t.token_type == ID || t.token_type == OUTPUT || t.token_type == INPUT){
        STATEMENT* statement = new STATEMENT();
        parse_statement(statement);
        // Check if the statement_list is empty (first statement to add)
        if (statement_list->head == nullptr) {
            // If it's empty, set the first statement as the head of the list
            statement_list->head = statement;
        } else {
            // Otherwise, find the last statement in the list and append the new one
            STATEMENT* current = statement_list->head;
            while (current->next != nullptr) {
                current = current->next;  // Traverse to the last statement
            }
            current->next = statement;  // Link the last statement to the new one
        }

        statement->next = nullptr;  // Ensure the new statement's next pointer is null
        parse_statement_list(statement_list);
    }
    else if (t.token_type == INPUTS){
        return;
    }
    else{
        cerr << "Unexpected token ahead in execute section: " << t.token_type << "\n";
        syntax_error();
        return;
    }
    return;
}
void Parser::parse_statement(STATEMENT* statement){
    //input, output, assign
    //make new statement malloc and pass to pasre x statement
    Token t = lexer.peek(1);
    if (t.token_type == INPUT){
        statement->statement_type = INPUT_STMT;
        parse_input_statement(statement);
        //return statement struct
        return;
    }
    else if (t.token_type == OUTPUT){
        statement->statement_type = OUTPUT_STMT;
        parse_output_statement(statement);
        //return statement struct
        return;
    }
    else if (t.token_type == ID){
        statement->statement_type = ASSIGN_STATEMENT;
        parse_assign_statement(statement);
        //return statement struct
        return;
    }
    else{
        cerr << "Unexpected token ahead in statement: " << t.token_type << "\n";
        syntax_error();
        return;
    }
    return;
}

void Parser::parse_input_statement(STATEMENT* statement){
    //input id semicolon
    //fill values of statement struct param
    Token t = expect(INPUT);
    Token k = expect(ID);
    Token j = expect(SEMICOLON);

    statement->LHS = -1;
    statement->poly_evaluation_t = nullptr;
    int success = grow_varmap(k.lexeme); 
    if (success < 0){
        cerr << "Error in growing var map with " << k.lexeme << " \n";
    }
    statement->var = get_varmap(k.lexeme);

    return;
}
void Parser::parse_output_statement(STATEMENT* statement){
    //output id semicolon
    //fill values of statement struct param
    Token t = expect(OUTPUT);
    Token k = expect(ID);
    Token j = expect(SEMICOLON);

    statement->LHS = -1;
    statement->poly_evaluation_t = nullptr;
    int success = grow_varmap(k.lexeme); 
    if (success < 0){
        cerr << "Error in growing var map with " << k.lexeme << " \n";
    }
    statement->var = get_varmap(k.lexeme);
    //return statement struct
    return;
}
void Parser::parse_assign_statement(STATEMENT* statement){
    //ID EQUAL poly_evaluation SEMICOLON
    //fill values of statement struct param
    Token t = expect(ID);
    int success = grow_varmap(t.lexeme); //add to var map
    if (success < 0){
        cerr << "Error in growing var map with " << t.lexeme << " \n";
    }
    Token k = expect(EQUAL);
    POLY_EVAL* poly_eval = new POLY_EVAL();
    statement->poly_evaluation_t = poly_eval;
    parse_poly_evaluation(poly_eval);
    Token j = expect(SEMICOLON);

    statement->var = -1;
    return;
}
void Parser::parse_poly_evaluation(POLY_EVAL* poly_eval){
    //poly_name LPAREN argument_list RPAREN
    poly_eval->name = (parse_poly_name()).lexeme;
    Token t = expect(LPAREN);
    ARGUMENT_LIST* arg_list = new ARGUMENT_LIST();
    poly_eval->argument_list = arg_list;
    arg_list->head = nullptr;
    parse_argument_list(arg_list);
    t = expect(RPAREN);
    t = lexer.peek(1);
    //comma or semicolon
    if (t.token_type == SEMICOLON){
        return;
    } else{ cerr << 
        "Unexpected token ahead in poly eval section: " << t.token_type << "\n";
        syntax_error();
        return;
    }
    return;
}
void Parser::parse_argument_list(ARGUMENT_LIST* argument_list){
    /*  argument
        argument COMMA argument_list
    */
    ARGUMENT* argument = new ARGUMENT();
    parse_argument(argument);
    Token t = lexer.peek(1);
    if (t.token_type == COMMA){
        Token k = expect(COMMA);
        parse_argument_list(argument_list);
    }
        //or end of poly eval
    else if (t.token_type == RPAREN){
        return;
    }
    return;

}
void Parser::parse_argument(ARGUMENT* argument){
    /*  ID
        NUM
        poly_evaluation -> poly_name -> ID
    */
    Token t = lexer.peek(1);
    Token k = lexer.peek(2);
    if (t.token_type == ID && k.token_type == LPAREN){
        POLY_EVAL* poly_eval = new POLY_EVAL();
        argument->poly_eval = poly_eval;
        parse_poly_evaluation(poly_eval);
        return;
    }
    else if (t.token_type == ID){
        t = expect(ID);
        argument->index = get_varmap(t.lexeme);
        argument->type = ID_TYPE;
        argument->poly_eval = nullptr;
        return;
    }
    else if (t.token_type == NUM){
        t = expect(NUM);
        argument->type = ID_TYPE;
        argument->value = stoi(t.lexeme);
        argument->poly_eval = nullptr;
        argument->index = -1;
        return;
    }
    else {
        cerr << "Unexpected token ahead when parsing argument: " << t.token_type << "\n";
        syntax_error();
        return;
    }
}
void Parser::parse_inputs_section(vector<int>* inputs_section){
    //INPUTS num_list
    //return int vector inputs_section
    Token t = expect(INPUTS);
    parse_num_list(inputs_section);
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

PROGRAM* Parser::allocate_program(){
    PROGRAM* program = new PROGRAM();
    program->tasks = new vector<int>;
    program->poly_section = new vector<POLY_DECL>;
    program->inputs_section = new vector<int>;
    program->execute_section = new STATEMENT_LIST();
    program->execute_section->head = nullptr;
    return program;
}

int main(){
    //load the file
    Parser parser; 
    PROGRAM* program = parser.allocate_program();
    //cout << "Starting parser\n";
    parser.parse_program(program);
    Token E_O_F = parser.expect(END_OF_FILE);
    cout << "Succesful parsing and on to task execution\n";
    task_execution(program);
    // Print tasks

    return 0;
}
    
    // note: the parser class has a lexer object instantiated in it. You should not be declaring
    // a separate lexer object. You can access the lexer object in the parser functions as shown in the
    // example method Parser::ConsumeAllInput
    // If you declare another lexer object, lexical analysis will not work correctly
