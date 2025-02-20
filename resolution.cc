#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cmath> //power
#include <algorithm>
#include <iostream>
#include "resolution.h"
#include "parser.h"
#include "lexer.h"
#include "inputbuf.h"

using namespace std;

/*
Global var decl
*/
static vector<pair<string, int> > var_map;
static int var_index = 0;
static vector<int> value_list;
int syntax_execution = 0;

//SEMANTIC CHECKING 
void task_execution(PROGRAM* program){
    //traverse the task list and execute 
    if(program->tasks){
        for (int task : *program->tasks){
            switch (task){
                case 1:
                //cout << "At task1 \n";
                if (task_1(program)){
                    return;
                }
                break;
            case 2:
                task_2(program);
                break;
            case 3:
                //handleTask3();
                break;
            case 4:
                //handleTask4();
                break;
            case 5: 
                //handleTask5()
            default:
                cout << "Unknown task: " << task << "\n";

            }
        }
    }else {
        cerr << "Tasks is found as nullptr in task execution";
    }
}
void task_2(PROGRAM* program){
    if (!program || !program->execute_section || !program->execute_section->head){
        cerr << "Null pointer found in task 2\n";
        return;
    }    
    int index = 0;
    cout << "Value list: \n";
    for (const auto& pair : value_list) {
        cout << index << " => " << pair << "\n";  
        index++;
    }
    //populate value list with inputs 
    for (const auto& value : *(program->inputs_section)){
        int success = set_valuelist(value);
        cout << "Added input " << value << " with return code " << success <<"\n";
        if (success < 0){cerr << "Error in growing value list\n"; return;}
    }
    cout << "Var list: \n";
    for (const auto& pair : var_map) {
        cout << pair.first << " => " << pair.second << "\n";  
    }
    cout << "Value list 2: \n";
    for (const auto& pair : value_list) {
        cout << index << " => " << pair << "\n";  
        index++;
    }

    STATEMENT* pc = program->execute_section->head;
    int v;
    while (pc){
        switch(pc->statement_type){
            case ASSIGN_STATEMENT:
                cout << "Resolving statement of polynomial " << pc->poly_evaluation_t->name << "\n";
                v = resolve_polynomial(pc->poly_evaluation_t, program->poly_section);
                cout << pc->poly_evaluation_t->name << " -> " << v << "\n";
            case INPUT_STMT:
                break;
            case OUTPUT_STMT:
                break;
        }
        pc = pc->next;
    }
    return;
}

int resolve_polynomial(POLY_EVAL* poly_eval, vector<POLY_DECL>* poly_section){
    if (!poly_eval) {
        cerr << "Null pointer found in evaluate polynomial\n";
        return -1;
    }
    /*STEP 1: get the poly declaration*/
    POLY_DECL* poly_decl = find_poly_decl(poly_section, poly_eval->name);
    if (!poly_decl) {
        cerr << "Null pointer returned by poly section in resolving poly no\n";
        return -1;
    }
    /* STEP 2 Resolve poly eval args */
    vector<pair<string, int>> arg_vals;
    
    if (poly_decl->poly_parameters) { // Ensure the pointer is not null
        for (const auto& param : *(poly_decl->poly_parameters)) {
            arg_vals.push_back({param.first, 0});
        }
    } else {
        arg_vals.push_back({"x", 0});
    }
    
    ARGUMENT* arg = poly_eval->argument_list ? poly_eval->argument_list->head : nullptr;
    int i = 0; // Track position in arg_vals
    while (arg && i < arg_vals.size()) {
        arg_vals[i].second = resolve_argument(arg, poly_section);
        cout << "Resolved argument, now arg_vals.size is " << arg_vals.size() << "\n";
        cout << arg_vals[i].first << " -> " << arg_vals[i].second << " is in arg_vals\n";
        arg = arg->next;
        i++;
    }

    if (arg_vals.empty()) {
        cerr << "No arguments resolved for polynomial evaluation\n";
        return -1;
    }
    
        cout << "Onto step 3\n";
    /* STEP 3: Compute the polynomial using resolved arguments */
    return resolve_polybody(poly_decl, arg_vals);
}
int resolve_argument(ARGUMENT* arg, vector<POLY_DECL>* poly_section){
    cout << "Resolving argument found on line no "<< arg->line_no<< " "<< arg->index<<"\n";
    if (!arg) {
        cerr << "Null argument node found in resolve_argument\n";
        return -1;
    }
    if (arg->type == NUM_TYPE) {
        return arg->value;  // Directly return the number
    } 
    else if (arg->type == ID_TYPE) {
        cout << "Arg index: " << arg->index << "\n";
        cout << "Value list returned: " << get_valuelist(arg->index) <<"\n";
        return get_valuelist(arg->index);  // Retrieve value from value list
    } 
    else if (arg->type == POLYEVAL_TYPE) {
        return resolve_polynomial(arg->poly_eval, poly_section);  // Recursive resolution
    } 
    else {
        cerr << "Unknown argument type in resolve_argument\n";
        return -1;
    }
}
int resolve_polybody(POLY_DECL* poly_decl, vector<pair<string, int>>& arg_vals){
    cout << "Resolving poly body \n";
    if (!poly_decl || !poly_decl->body->head) {
        cerr << "Null poly declaration or body in resolve poly body\n";
        return -1;
    }
    int value = 0;
    /*resolve term by term and collect values*/
    TERM* term = poly_decl->body->head;
    value = resolve_term(term, arg_vals);
    if (term->addop == NONE){
        return value; 
    } else if (term->addop == PLUS_SIGN){
        value= value + resolve_term(term->next, arg_vals);
        cout << "Value at resolve poly_body is increased " << value << "\n";
    }else if (term->addop == MINUS_SIGN){
        value-= resolve_term(term->next, arg_vals);
        cout << "Value at resolve poly_body is decreased " << value << "\n";

    }
    return value;
}
int resolve_term(TERM* term, vector<pair<string, int>>& arg_vals){
    cout << "Resolving term \n";
    if (!term){
        cerr << "Null term or monom list declaration in resolve term\n";
        return -1;
    }
    int value = 0;
    /*STEP 1 resolve monom list*/
    if (term->monomial_list){
        MONOMIAL* monom = term->monomial_list->head;
        while(monom){
            value = resolve_monomial(monom, arg_vals);
            cout << "Value at resolve term is " << value << "\n";
            monom = monom->next;
        }
    }
    /*STEP 2 resolve coefficient*/
    if (value == 0){return term->coefficient;}
    value = value * term->coefficient;
    return value;
}
int resolve_monomial(MONOMIAL* monom, vector<pair<string, int>>& arg_vals){
    cout << "Resolving monomial \n";
    if (!monom || !monom->primary){
        cerr << "Null monom or primary declaration in resolve monomial\n";
        return -1;
    }
    int value = 0;
    /*STEP 1 resolve primary */
    PRIMARY* primary = monom->primary;
    value = resolve_primary(primary, arg_vals);
    cout << "Value at resolve monom is " << value << "\n";
    /*STEP 2 resolve exponent application*/
    if (monom->exponent == 0){value = 1;}
    else if (monom->exponent != 1){value = pow(value, monom->exponent);} 
    return value;
}
int resolve_primary(PRIMARY* primary, vector<pair<string, int>>& arg_vals){
    int value = 0;
    cout << "Resolving primary\n";
    /*STEP 1 identify type of primary ID or term_list */
    if (primary->type == IDENFIER){
        cout << primary->lexeme <<" is primary lexeme found\n";
        cout << primary->var_index <<" is primary var index found\n";
        for (const auto& pair : arg_vals) {
            if (pair.first == primary->lexeme) {
                cout <<"Returned " << pair.second <<" \n";
                return pair.second;
            }
        }
        cerr << "Error: Lexeme " << primary->lexeme << " not found in arg_vals\n";
        return -1; // Return an error value if lexeme is not found
    } else if (primary->type == RTERML){
        if (!primary->term_list->head){
            cerr << "Primary with term list and no term list in monom found!";
        }
        TERM* term = primary->term_list->head;
        while (term){
            value = resolve_term(term, arg_vals);
            term = term->next;
        }
    }
    else {cerr << "Something happened in primary :/\n";}
    return value;
}
bool task_1(PROGRAM* program){
    //syntax handling
    int status = syntax_execution_get(); 
    if (status > 0) {
        //cout << "SYNTAX ERROR !!!!!&%!!\n";
    }
    /*
    SEMANTIC HANDLING 
    */
   if (code_1(program->poly_section)) return true;
   if (code_2(program->poly_section)) return true;
   if (code_3(program)) return true;
   if (code_4(program)) return true;   
   else {
    return false;
    //cout << "No semantic errors found\n";
   }
}

bool code_1(vector<POLY_DECL>* poly_section){
    if (!poly_section) {
        cerr << "Empty poly section found at code 1\n";
        return false;
    }
    unordered_map<string, int> occurrences;
    vector<int> duplicates;

    for (const auto& poly : *poly_section){
        if (occurrences.count(poly.name)){
            duplicates.push_back(poly.line_no); //if there is the poly name, mark its duplicate
        } else {
            occurrences[poly.name] = poly.line_no; //store its occurence and line no in hash
        }
    }
    if (!duplicates.empty()){
        cout << "Semantic Error Code 1:";
        for (int line : duplicates){
            cout << " " << line; //?? maybe already sorted
        }
        cout << "\n";
        return true;
    }
    return false;
}
bool code_2(vector<POLY_DECL>* poly_section){
    vector<int> invalid_line;
  
    //for every poly nomial in the poly section
    for (const auto& poly : *poly_section){
            //make a list of its polynomial variables (arguments taken)
            vector<string> poly_vars;
            for (const auto& param : *poly.poly_parameters) {
                //cout << "Parameter " << param.first << " collected \n";
                poly_vars.push_back(param.first); // Collect variable names (like x, y, etc.)
            }
            if (poly.body) {
                invalid_line = process_term_list(poly.body, invalid_line, poly_vars);
            } else {
                cerr << "No poly boday found in code 2\n";
            }
        }
    if (!invalid_line.empty()) {
        cout << "Semantic Error Code 2:";
        for (int line_no : invalid_line) {
            cout << " " << line_no;
        }
        cout << '\n';
        return true; //error occurred
    }

    return false;
}

vector<int> process_term_list(TERM_LIST* term_list, vector<int> invalid_line, const vector<string>& poly_vars) {
    TERM* term = term_list ? term_list->head : nullptr;
    if (!term){cerr << "No term list \n";}
    while (term) {
        MONOMIAL* monomial = term->monomial_list ? term->monomial_list->head : nullptr;
        while (monomial) {
            if (monomial->primary->type == IDENFIER) {
                bool is_valid = false;
                for (const string& var : poly_vars) {
                    if (monomial->primary->lexeme == var) {
                        is_valid = true;
                        break;  // Exit loop early since we found a match
                    }
                }
                if (!is_valid) {  // If no match was found, mark it as invalid
                    invalid_line.push_back(monomial->primary->line_no);
                    //cout << "No match found for var " << monomial->primary->lexeme << ", adding line " << monomial->primary->line_no << " to invalid list.\n";
                }
            } else if (monomial->primary->type == RTERML){
                //cout << monomial->primary->line_no << " \n";
                invalid_line = process_term_list(monomial->primary->term_list, invalid_line, poly_vars);
            }
        monomial = monomial->next;
        }
        term = term->next; // Move to the next term
    }
    return invalid_line; // Return the list of invalid lines
}
bool code_3(PROGRAM* program){
    /*
    for every statement in statement list execute section
        its poly-eval->name must exist in program->poly_decl table else error
    */
   if (!program || !program->execute_section || !program->poly_section){
    cerr << "Null pointer encountered in code 3\n";
    return false;
   }
   vector<int> error_lines;
   unordered_set<string> polyNames; // will hold 
   for (const POLY_DECL& polyDecl : *program->poly_section) {
    polyNames.insert(polyDecl.name);
    }   

    //iterate through statements in execute section
    STATEMENT* currentStmt = program->execute_section->head;
    while (currentStmt) {
        //if cur stmt has a poly eval, check it 
        if (currentStmt->poly_evaluation_t) {
            error_lines = check_poly_eval(currentStmt->poly_evaluation_t, polyNames, error_lines);
        }
        currentStmt = currentStmt->next; // Move to the next statement
    }
    if (!error_lines.empty()){
        cout << "Semantic Error Code 3:";
        for (int line : error_lines){
            cout << " " << line; //?? maybe already sorted
        }
        cout << "\n";
        return true;
    }
    // All polynomial evaluation names exist, return true
    return false;
}
vector<int> check_poly_eval(POLY_EVAL* poly_eval, unordered_set<string> polyNames, vector<int> error_lines){
    string polyEvalName = poly_eval->name;
    //cout << "Checking name: " << polyEvalName << " \n";
    if (polyNames.find(polyEvalName) == polyNames.end()) {
        //cout << "Pushing " << poly_eval->name << " with line no " << poly_eval->line_no << "\n";
        error_lines.push_back(poly_eval->line_no);
    }
    if (poly_eval->argument_list){
        //cout << "Checking name: " << polyEvalName << " argument list \n";
        error_lines = check_arg_list(poly_eval->argument_list, polyNames, error_lines);
    }
    return error_lines;
}
vector<int> check_arg_list(ARGUMENT_LIST* argument_list,unordered_set<string> polyNames, vector<int> error_lines){
    if (!argument_list || !argument_list->head) {
        cerr << "Null pointer encountered in arg list\n";
        return error_lines; // Check for nullptr
    }
    ARGUMENT* argument = argument_list->head;
    while (argument){
        if(argument->type == NUM_TYPE || argument->type == ID_TYPE){
            //cout << "Argument type number or id encountered. Returning \n";
            argument = argument->next;
            continue;
        }
        else{
            //cout << "Argument type assign encountered. Checking poly eval \n";
            error_lines = check_poly_eval(argument->poly_eval, polyNames, error_lines);
        }
        argument = argument->next;
    }
    return error_lines;
}
bool code_4(PROGRAM* program){
    vector<int> error_lines; 
    if (!program->execute_section || !program->execute_section->head){
        cerr << "Null pointer found in code 4\n";
        return false;
    }
    STATEMENT* statement = program->execute_section->head;

    while(statement){
        if (statement->statement_type == ASSIGN_STATEMENT || statement->poly_evaluation_t){
            POLY_DECL* poly_decl = find_poly_decl(program->poly_section, statement->poly_evaluation_t->name);
            if (poly_decl){
                //cout << "Reviewing " << poly_decl->name << " with poly eval " << statement->poly_evaluation_t->name<<" \n";
                error_lines = compare_argument_count(program->poly_section, poly_decl, statement->poly_evaluation_t, error_lines); //how to pass poly decl        
            }
        }
        statement = statement->next;
    }
    if (!error_lines.empty()){
        cout << "Semantic Error Code 4:";
        sort(error_lines.begin(), error_lines.end()); // Sort in ascending order
        for (int line : error_lines){
            cout << " " << line; //?? maybe already sorted
        }
        cout << "\n";
        return true;
    }
    return false;
}
POLY_DECL* find_poly_decl(vector<POLY_DECL>* poly_section, const string& name) {
    if (!poly_section) return nullptr;
        for (auto& poly_decl : *poly_section){
            if (poly_decl.name == name){
                return &poly_decl;
                }
        }
    return nullptr;
}
vector<int> compare_argument_count(vector<POLY_DECL>* poly_section, POLY_DECL* poly_decl, POLY_EVAL* poly_eval, vector<int> error_lines){
    if (!poly_decl || !poly_eval) return error_lines; // Safety check
    //how many expected
    int expected_args = (poly_decl->poly_parameters) ? poly_decl->poly_parameters->size() : 1;
    //cout << "Expected args for poly decl " << poly_decl->name << " "<< expected_args << " \n";
    int eval_args = 0;
    ARGUMENT* argument = (poly_eval->argument_list) ? poly_eval->argument_list->head : nullptr;
   //cout << "Argument pointer made\n";
    while (argument) {
        eval_args++; //collect argument num
        if (argument->type == POLYEVAL_TYPE|| argument->poly_eval){
            POLY_DECL* poly_decl = find_poly_decl(poly_section, argument->poly_eval->name);
            if (poly_decl){
                //cout << "Reviewing " << poly_decl->name << " with poly eval " << argument->poly_eval->name <<" \n";
                error_lines = compare_argument_count(poly_section, poly_decl,argument->poly_eval , error_lines); //how to pass poly decl        
            }
        }
        argument = argument->next;
    }
    if (expected_args != eval_args){
        //cout << "Eval args for poly eval " << poly_eval->name << " are " << eval_args << " . Pushing to error lines\n";
        error_lines.push_back(poly_eval->line_no);
    }
    return error_lines;
}

//SYNTAX getter and setter
int syntax_execution_get(){
    return syntax_execution;
}
void syntax_execution_set(int status){
    syntax_execution = status;
}

/*
This manages the variable map and the value list
*/

int grow_varmap(string lexeme){
    //check if lexeme already exists in var map
    for (size_t i = 0; i < var_map.size(); i++){
        if(var_map[i].first == lexeme){
            return var_map[i].second;
        }
    }
    // If lexeme is new, add it to var_map with the next available index
    var_map.push_back({lexeme, var_index});
    return var_index++;
}

int get_varmap(string lexeme){
    for (const auto& entry: var_map){
        if (entry.first == lexeme){
            return entry.second;
        }
    }
    return -1; // Error value
}
string get_lexeme(int value) {
    for (const auto& entry : var_map) {
        if (entry.second == value) {
            return entry.first;
        }
    }
    cerr << "Error: Value '" << value << "' not found!\n";
    return ""; // Error value
}

int get_valuelist(int index){
    if (index < 0 || index >= value_list.size()) {
        return -1;  // Error: Index out of bounds
    }
    return value_list[index];
}
int set_valuelist(int value){
    value_list.push_back(value);
    return value_list.size() - 1; //returns the new index of the value
}
/*
Debugging print stmts
*/
void printTasks(PROGRAM* program){
    cout << "TASKS:\n";
    for (size_t i = 0; i < program->tasks->size(); i++) {
        printf("TASK %zu: %d\n", i, (*program->tasks)[i]);
    }
}
void printPOLYDECL(PROGRAM* program) {
    for (const auto& poly : *program->poly_section) {
        print_poly_decl(poly);
    }
}
// Function to print the polynomial body terms (terms with monomials)
void print_term_list(TERM_LIST* term_list) {
    TERM* term = term_list ? term_list->head : nullptr;
    while (term) {
        cout << "  Coefficient: " << term->coefficient << ", Add Operator: " << term->addop << endl;
        
        MONOMIAL* monomial = term->monomial_list ? term->monomial_list->head : nullptr;
        while (monomial) {
            cout << "    Exponent: " << monomial->exponent << ", Primary Type: " << monomial->primary->type;
            if (monomial->primary->type == IDENFIER) {
                cout << ", Var Index: " << monomial->primary->var_index;
            }
            cout << endl;
            monomial = monomial->next;
        }
        
        term = term->next;
    }
}

// Function to print polynomial declaration details
void print_poly_decl(const POLY_DECL& poly) {
    cout << "Polynomial Name: " << poly.name << endl;
    cout << "Line Number: " << poly.line_no << endl;

    // Print polynomial parameters
    if (poly.poly_parameters != nullptr) {
        cout << "Polynomial Parameters: ";
        for (const auto& param : *poly.poly_parameters) {
            cout << param.first << ":" << param.second << " ";
        }
        cout << endl;
    }

    // Print body (if it exists)
    if (poly.body != nullptr) {
        cout << "Body of the Polynomial:" << endl;
        print_term_list(poly.body);
    } else {
        cout << "No body for this polynomial." << endl;
    }
    cout << "------------------------------------" << endl;
}

void printINPUTS(PROGRAM* program){
    // Print inputs
    cout << "INPUTS:\n";
    for (size_t i = 0; i < program->inputs_section->size(); i++) {
        printf("INPUT %zu: %d\n", i, (*program->inputs_section)[i]);
    }
}

void printEXECUTE(PROGRAM* program){
    if (program->execute_section) {
        STATEMENT* current = program->execute_section->head; // Start from the head
        size_t i = 0;
        while (current) { // Traverse the linked list
            printf("STATEMENT %zu: Type %d, Var %d, LHS %d\n", 
                i, current->statement_type, current->var, current->LHS);
            current = current->next; // Move to the next statement
            i++;
            }
        } 
    else {
        cout << "No EXECUTE section.\n";
    }
}