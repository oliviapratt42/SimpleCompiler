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
#include "resolution.h"
#include "parser.h"
#include "lexer.h"
#include "inputbuf.h"

using namespace std;


void Parser::syntax_error()
{
    cout << "SYNTAX ERROR !!!!!&%!!\n";
    syntax_execution_set(1); //send saying execution happened
    exit(1);
}

PROGRAM* Parser::parse_program(PROGRAM* program){
    //tasks_section poly_section execute_section inputs_section

    parse_tasks_section(*(program->tasks));
    //printTasks(program);

    parse_poly_section(program->poly_section);
    //printPOLYDECL(program);

    parse_execute_section(program->execute_section);
   // printEXECUTE(program);
    parse_inputs_section(*(program->inputs_section));

    Token t = expect(END_OF_FILE);
    //printINPUTS(program);
    return program;
}

void Parser::parse_tasks_section(vector<int>& tasks){
    //TASKS num_list
    Token t = expect(TASKS);
    parse_num_list(tasks);
    return;
}
void Parser::parse_num_list(vector<int>& nums){
    //NUM
    //NUM num_list
    Token t = expect(NUM);
    nums.push_back(stoi(t.lexeme));//grow num list

    Token s = lexer.peek(1);
    if (s.token_type == NUM){ 
        parse_num_list(nums); //recursive
    }
}

void Parser::parse_poly_section(vector<POLY_DECL>* poly_declarations){
    //POLY poly_decl_list
    Token k = expect(POLY);
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
    return;
}
void Parser::parse_poly_decl(vector<POLY_DECL>* poly_declarations){
    //poly_header EQUAL poly_body SEMICOLON
    POLY_DECL* poly_decl = new POLY_DECL();
    if (!poly_decl) {
        cerr << "Memory allocation failed for POLY_DECL" << endl;
        return;
    }     //cout << "Parsing poly declaration\n";
    parse_poly_header(poly_decl); //has an ID check already 
    Token k = expect(EQUAL);
    parse_poly_body(poly_decl);
    Token s = expect(SEMICOLON);
    poly_declarations->push_back(*poly_decl); //adding to table
}

//debugged
void Parser::parse_poly_header(POLY_DECL* poly_decl){
    //poly_name
    //s is an ID meaning poly name found
    Token name = parse_poly_name();
    //cout << "Parsing poly header "<<name.lexeme<<"\n";
    poly_decl->name = name.lexeme; // add name to poly decl
    poly_decl->line_no = name.line_no; //add line no of poly decl
    Token t = lexer.peek(1);
    //if has arguments
    if (t.token_type == LPAREN){
        Token k = expect(LPAREN);
        int order = 0;
        poly_decl->poly_parameters = new vector<pair<string, int> >();
        parse_id_list(poly_decl->poly_parameters, order); 
        Token j = expect(RPAREN);
    } 
    //else no arguments 
    else if (t.token_type == EQUAL){
        poly_decl->poly_parameters = new vector<pair<string, int> >();
        poly_decl->poly_parameters->emplace_back("x", 0);
        return;
    }
    return;
}

void Parser::parse_id_list(vector<pair<string, int> >* poly_parameters, int order){
    //ID
    //ID COMMA id_list
    Token t = expect(ID);
    poly_parameters->push_back(make_pair(t.lexeme, order)); 

    Token k = lexer.peek(1);
    if (k.token_type == COMMA){
        Token s = expect(COMMA);
        order++; //there is another one
        Token j = lexer.peek(1);
        //if another id in list, id must exist after comma
        if (j.token_type == ID){
            parse_id_list(poly_parameters, order);
        } else {
            syntax_error();
        }
    }
    return;
}
Token Parser::parse_poly_name(){
    //ID
    Token t = expect(ID);
    Token k = lexer.peek(1);
    if (k.token_type == LPAREN || k.token_type == EQUAL){return t;}
    return t;
}
void Parser::parse_poly_body(POLY_DECL* poly_decl){
    //term_list
    TERM_LIST* term_list = new TERM_LIST();
    poly_decl->body = term_list;
    //semicolon shwould follow poly body 
    parse_term_list(term_list);
    return;
}
void Parser::parse_term_list(TERM_LIST* term_list){
    //term
    //term add_operator term_list
    TERM* term = new TERM();
    parse_term(term);

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
    //term lists can be followed by add operators meaning ne term 
    Token t = lexer.peek(1);
    if (t.token_type == PLUS || t.token_type == MINUS){
        term->addop = parse_add_operator();
        parse_term_list(term_list);
        }

    else if (t.token_type == SEMICOLON|| t.token_type == RPAREN){
        //end of the term list
        term->addop = NONE;
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
        Token k = lexer.peek(1);
        if (k.token_type == ID || k.token_type == LPAREN){
            //create new monomial list
            MONOMIAL_LIST* monomial_list = new MONOMIAL_LIST();
            monomial_list->head = nullptr;
            term->monomial_list = monomial_list;
            parse_monomial_list(monomial_list);
        }
    }
    else{
        syntax_error();
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
        if (!monomial_list->head){
            monomial_list->head = new_monomial;
        }else{
            MONOMIAL* curr = monomial_list->head;
            while (curr->next){
                curr = curr->next;
            }
            curr->next = new_monomial;
        }
        new_monomial->next = nullptr;
        Token k = lexer.peek(1);
        if (k.token_type == ID || k.token_type == LPAREN){
            //if there is another monomial, we want to continue parsing this list
            parse_monomial_list(monomial_list);
        }
    }
    //monomial list follow: add operator, semicolon, rparen
    else if (t.token_type == PLUS || t.token_type == MINUS || t.token_type == SEMICOLON || t.token_type == RPAREN){
        return;
    } else {
        //cerr << "Unexpected token at parse monom list " << t.token_type << " \n"; syntax_error();
        return;
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
    else if (t.token_type == LPAREN || t.token_type == ID || t.token_type == SEMICOLON || t.token_type == PLUS || t.token_type == MINUS || t.token_type == RPAREN){
        monomial->exponent = 1;
    } else { 
        //cerr << "Unexpected token at parse monom list " << t.token_type << " \n"; syntax_error()
        return;}
    return;
}
void Parser::parse_primary(PRIMARY* primary){
    //ID or LParent term_list RParent
    Token t = lexer.peek(1);
    if (t.token_type == ID){
        Token k = expect(ID);
        primary->type = IDENFIER;
        primary->line_no = t.line_no;
        primary->lexeme = t.lexeme;
        return;
    }
    else if(t.token_type == LPAREN){
        Token k = expect(LPAREN);
        TERM_LIST* new_term_list = new TERM_LIST();
        parse_term_list(new_term_list);
        primary->term_list = new_term_list;
        Token j = expect(RPAREN);
        primary->type = RTERML;
        return;
    }else {
        //cerr << "Unexpected token at parse primary" << t.token_type << " \n";syntax_error(); 
        return;}
}
int Parser::parse_exponent(){
    //POWER NUM
    Token t = expect(POWER);
    Token k = expect(NUM);
    int value = stoi(k.lexeme); 
    k = lexer.peek(1);
    if (k.token_type == PLUS ||k.token_type == MINUS ||k.token_type == LPAREN ||k.token_type == SEMICOLON||k.token_type == RPAREN || k.token_type == ID){return value;}
    else {
        //cerr << "Unexpected token at parse exponent " << k.token_type << " \n";syntax_error();
        return -1;    
    }
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
        //cerr << "Unexpected token ahead when parsing add operator: " << t.token_type << "\n";
        //syntax_error();
        return NONE;
    }    
    return NONE;
}
int Parser::parse_coefficient(){
    //NUM
    Token t = expect(NUM);
    int value = stoi(t.lexeme);
    //FIXME int value = (int)t.lexeme;
    Token k = lexer.peek(1);
    if (k.token_type == PLUS || k.token_type == MINUS|| k.token_type == ID || k.token_type == LPAREN || k.token_type == RPAREN || k.token_type == SEMICOLON){
        return value;
    }
    else {//cerr << "Unexpected token at parse coefficient " << k.token_type << " \n"; syntax_error(); 
        return -1;
    }

}
void Parser::parse_execute_section(STATEMENT_LIST* statement_list){
    //EXECUTE statementnt_list
    Token t = expect(EXECUTE);
    Token k = lexer.peek(1);
    if (k.token_type == INPUT || k.token_type == OUTPUT || k.token_type == ID) {
        parse_statement_list(statement_list);
    }
    else {cerr <<  "Unexpected token at execute section " << k.token_type << " \n"; syntax_error();
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
        if (statement_list->head == nullptr) {
            statement_list->head = statement;
        } else {
            STATEMENT* current = statement_list->head;
            while (current->next != nullptr) {
                current = current->next;  // Traverse to the last statement
            }
            current->next = statement;  // Link the last statement to the new one
        }
        statement->next = nullptr;  // Ensure the new statement's next pointer is null
        Token k = lexer.peek(1);
        //anticipate another statement else end of section
        if (k.token_type == ID || k.token_type == OUTPUT || k.token_type == INPUT){
            parse_statement_list(statement_list);
        } else if (k.token_type == INPUTS){
            return;
        }
    }
    else{//cerr << "Unexpected token in execute section: " << t.token_type << "\n"; syntax_error();
        return;    
    }    
    return;
}
void Parser::parse_statement(STATEMENT* statement){
    //input, output, assign
    //make new statement malloc and pass to pasre x statement
    Token t = lexer.peek(1);
    statement->line_no = t.line_no;
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
        //cerr << "Unexpected token ahead in statement: " << t.token_type << "\n";
        //syntax_error();
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
    statement->lexeme = k.lexeme;
    statement->poly_evaluation_t = nullptr;
    int success = grow_varmap(k.lexeme);
    if (success != -1){
        statement->var = success;
        variables[statement->var] = 1;
    }
    return;
}
void Parser::parse_output_statement(STATEMENT* statement){
    //output id semicolon
    //fill values of statement struct param
    Token t = expect(OUTPUT);
    Token k = expect(ID);
    Token j = expect(SEMICOLON);
    statement->lexeme = k.lexeme;
    statement->poly_evaluation_t = nullptr;
    int exists = -1;
    if (exists != -1){statement->var = get_varmap(k.lexeme);}
    //return statement struct
    return;
}
void Parser::parse_assign_statement(STATEMENT* statement){
    //ID EQUAL poly_evaluation SEMICOLON
    //fill values of statement struct param
    Token t = expect(ID);

    Token k = expect(EQUAL);
    statement->lexeme = t.lexeme;
    POLY_EVAL* poly_eval = new POLY_EVAL();
    parse_poly_evaluation(poly_eval);

    int success = get_varmap(t.lexeme);
    if (success == -1){statement->var = grow_varmap(t.lexeme);}
    else{statement->var = success;}

    statement->poly_evaluation_t = poly_eval;
    Token j = expect(SEMICOLON);
    return;
}
void Parser::parse_poly_evaluation(POLY_EVAL* poly_eval){
    //poly_name LPAREN argument_list RPAREN
    Token lexeme = parse_poly_name();
    poly_eval->name = lexeme.lexeme;
    poly_eval->line_no = lexeme.line_no;
    Token t = expect(LPAREN);
    ARGUMENT_LIST* arg_list = new ARGUMENT_LIST();
    poly_eval->argument_list = arg_list;
    parse_argument_list(arg_list);
    t = expect(RPAREN);
    t = lexer.peek(1);
    //comma or semicolon
    if (t.token_type == SEMICOLON || t.token_type == RPAREN || t.token_type == COMMA){
        return;
    } else{ //cerr << 
        //"Unexpected token ahead in poly eval section: " << t.token_type << "\n";
        //syntax_error();
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
    if (!argument_list->head){argument_list->head = argument;}
    else{
        ARGUMENT* currArg = argument_list->head;
        while (currArg->next){
            currArg = currArg->next;
        }
        currArg->next = argument;
    }
    argument->next = nullptr;
    
    Token j = lexer.peek(1);
    if (j.token_type == COMMA){
        Token k = expect(COMMA);
        Token m = lexer.peek(1);
        if (m.token_type == ID || m.token_type == NUM){
            parse_argument_list(argument_list);
        } else {cerr << "Unexpected token type " << m.token_type << " found in argument list\n"; syntax_error();
            return;
        }
    } else if(j.token_type == RPAREN){return;} //end of argument list 
    else {//cerr << "Unexpected token type " << j.token_type << " found in argument list\n"; syntax_error();
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
        argument->line_no = t.line_no;
        argument->type = POLYEVAL_TYPE;
    }
    else if (t.token_type == ID){
        t = expect(ID);
        argument->index = get_varmap(t.lexeme);
        argument->type = ID_TYPE;
        argument->lexeme = t.lexeme;
        argument->poly_eval = nullptr;
        argument->line_no = t.line_no;
    }
    else if (t.token_type == NUM){
        t = expect(NUM);
        argument->type = NUM_TYPE;
        argument->value = stoi(t.lexeme);
        argument->poly_eval = nullptr;
        argument->index = -1;
        argument->line_no = t.line_no;
    }
    else {
        cerr << "Unexpected token ahead when parsing argument: " << t.token_type << "\n";
        syntax_error();
    }
    return;
}
void Parser::parse_inputs_section(vector<int>& inputs_section){
    //INPUTS num_list
    //return int vector inputs_section
    Token t = expect(INPUTS);
    parse_num_list(inputs_section);
    Token k = lexer.peek(1);
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
    task_execution(program);
    // Print tasks

    return 0;
}
    
    // note: the parser class has a lexer object instantiated in it. You should not be declaring
    // a separate lexer object. You can access the lexer object in the parser functions as shown in the
    // example method Parser::ConsumeAllInput
    // If you declare another lexer object, lexical analysis will not work correctly
