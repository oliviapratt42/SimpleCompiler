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
static vector<string> var_map;
static int var_index = 0;
int syntax_execution = 0;
static int mem[1000] = {0};     
int variables[1000] = {0};

vector <int> warning_var;
vector<pair<string, bool>> var_use; 
vector<pair<string, int>> use_lines;



//SEMANTIC CHECKING 
void task_execution(PROGRAM* program){
    //traverse the task list and execute 
        /*
   cout << "Var list: \n";
   for (const auto& pair : var_map) {
       cout << pair.first << " => " << pair.second << "\n";  
   }
   int index = 0;
   cout << "Value list: \n";
   for (const auto& pair : value_list) {
       cout << index << " => " << pair << "\n";  
       index++;
   }
   */
  process_inputs(program->inputs_section);
  vector<int>* tasks = program->tasks;
  sort(tasks->begin(), tasks->end());
  tasks->erase(unique(tasks->begin(), tasks->end()), tasks->end());
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
                task_3(program);
                break;
            case 4:
                task_4(program);
                break;
            case 5: 
                task_5(program);
                break;
            default:
                //cout << "Unknown task: " << task << "\n";
                break;
            }
        }
    }else {
        cerr << "Tasks is found as nullptr in task execution";
    }
}
void task_5(PROGRAM* program){
    vector<pair<string, int>> polyNames;
    for (const POLY_DECL& polyDecl : *(program->poly_section)) {
        polyNames.emplace_back(polyDecl.name, 0);
    }
    for (POLY_DECL& polyDecl : *(program->poly_section)){
        int v = count_poly(&polyDecl);
        for (auto& entry : polyNames) {
            if (entry.first == polyDecl.name) {
                entry.second = v; // Update with computed value
                break; // Exit the loop once found
            }
        }
    }
    for (const auto& entry : polyNames){
        cout << entry.first << ": " << entry.second << endl;
    }
}
int count_poly(POLY_DECL* poly_decl){
    int value = 0;
    if (poly_decl->body){value = count_term(poly_decl->body->head);}
    return value;
}
int count_term(TERM* term){
    int value = 0;
    if(term->monomial_list){
        value = count_monomial(term->monomial_list->head);
        //cout <<"Term degree is " << value << endl;
    }
    if(term->next){
        int val2 = count_term(term->next);
        if (val2 > value){value = val2;}
    }
    return value;
}
int count_monomial(MONOMIAL* monom){
    int value = count_primary(monom->primary);
    value *= monom->exponent;
    if (monom->next){
        value += count_monomial(monom->next);
    }
    //cout <<"Monom degree is " << value << endl;
    return value;
}
int count_primary(PRIMARY* primary){
    if (primary->type == IDENFIER){
        return 1;
    } else {
        return count_term(primary->term_list->head);
    }
}
void task_4(PROGRAM* program){
    STATEMENT* statement = program->execute_section->head;

    while (statement){
        if (statement->statement_type == ASSIGN_STATEMENT){
            check_var_use(statement->poly_evaluation_t->argument_list);
            var_use.push_back({statement->lexeme, false});
            use_lines.push_back({statement->lexeme, statement->line_no});

        } else if (statement->statement_type == INPUT_STMT){
            update_use(statement->lexeme, false);
            var_use.push_back({statement->lexeme, false});            
            use_lines.push_back({statement->lexeme, statement->line_no});
        } else if (statement->statement_type == OUTPUT_STMT){
            update_use(statement->lexeme, true);
        }
        statement = statement->next;
    }
    vector<int> lines;
    for (size_t i = 0; i < use_lines.size(); ++i){
        int line_no = use_lines[i].second;
        if (!var_use[i].second){
            lines.push_back(line_no);
        }
    }

    if (!lines.empty()){
        cout << "Warning Code 2:";
        for (int line : lines){
            cout << " " << line;
        }
        cout << "\n";
    } else{
        return;
    }
}

void check_var_use(ARGUMENT_LIST* arg_list){
    ARGUMENT* arg = arg_list->head;
    while (arg){
        if (arg->type == ID_TYPE){
            update_use(arg->lexeme, true);
        } else if (arg->type == POLYEVAL_TYPE){
            check_var_use(arg->poly_eval->argument_list);
        }
        arg = arg->next;
    }
}

void update_use(string lexeme, bool status){
    for (auto it = var_use.rbegin(); it != var_use.rend(); ++it){
        if (it->first == lexeme){
            it->second = status;
            break;
        }
    }
}

void task_3(PROGRAM* program){
  //check evaluation variables
  STATEMENT* statement = program->execute_section->head;
  while (statement){
    if (statement->statement_type == ASSIGN_STATEMENT){                          
        validate_decl(statement->poly_evaluation_t->argument_list);
    }
    statement = statement->next;
    }

    cout << "Warning Code 1:";
    sort(warning_var.begin(), warning_var.end());
    for (const auto& line : warning_var){
     cout  << " " << line;
    }
    cout << "\n";

}
void validate_decl(ARGUMENT_LIST* arg_list){
    ARGUMENT* arg = arg_list->head;
    while(arg){
        if (arg->type == ID_TYPE){
            if (arg->index == -1){
                add_to_warning(arg->line_no);
            }            
        } else if (arg->type == POLYEVAL_TYPE){
            validate_decl(arg->poly_eval->argument_list);
        }
        arg = arg->next;
    }
    return;
}
void add_to_warning(int line){
    warning_var.push_back(line);
}

void process_inputs(vector<int>* inputs) {
    int mem_index = 0;  // Track valid memory insertions
    size_t input_index = 0; // Track progress in inputs list
    for (int j = 0; j < 1000; j++) {
        if (input_index >= inputs->size()) {
            break; // Stop if we've used all inputs
        }else{
        if (variables[j] == 1) {
            mem[mem_index] = inputs->at(input_index);
            input_index++; // Move to next input only when used
        } mem_index++; // Always move mem forward

    }
}
}


void task_2(PROGRAM* program){
    if (!program || !program->execute_section || !program->execute_section->head){
        cerr << "Null pointer found in task 2\n";
        return;
    }    
    STATEMENT* pc = program->execute_section->head;
    //POLY_DECL* polyDecl;
    while (pc){
        if (pc->statement_type == ASSIGN_STATEMENT){
            cout << "Found assign stmt on line " << pc->line_no << endl;
            POLY_DECL* poly_decl = find_poly_decl(program->poly_section, pc->poly_evaluation_t->name);
            int v = resolve_polynomial(pc->poly_evaluation_t, poly_decl);
            mem[pc->var] = v; //update 
            cout << "Resolved to " << mem[pc->var] << endl;
        }
        else if (pc->statement_type == OUTPUT_STMT){
            if (pc->var == -1){
                cout << "0\n"; //KEEP ALWAYS
            } 
            else{
            int v = mem[pc->var];
            //cout << pc->var << endl;
            cout << v << "\n";
        }
            }
            
        pc = pc->next;
    }
    return;
}

int resolve_polynomial(POLY_EVAL* poly_eval, POLY_DECL* poly_decl){
    if (!poly_eval) {
        cerr << "Null pointer found in evaluate polynomial\n";
        return -1;
    }
    if (!poly_decl) {
        cerr << "Null pointer returned by poly section in resolving poly no\n";
        return -1;
    }
    vector<int> arg_vals;
    cout << "For poly " << poly_decl->name <<endl;
    ARGUMENT* argument = poly_eval->argument_list->head;
    int times = 0;
    while (argument){
        cout << "Passed an arg " << times << " times" << endl;
        int argument_value = resolve_argument(argument);
        arg_vals.push_back(argument_value);
        argument = argument->next;
    }
    /* STEP 4: Compute the polynomial using resolved arguments */
    return resolve_term(poly_decl->body->head, arg_vals);
}
int resolve_argument(ARGUMENT* arg){
    //ERASE
    //cout << "Resolving argument found on line no "<< arg->line_no<< " "<< arg->index <<"\n";
    if (!arg) {
        cerr << "Null argument node found in resolve_argument\n";
        return -1;
    }
    if (arg->type == NUM_TYPE) {
        //ERASE
        cout << arg->value << endl;
        return arg->value;  // Directly return the number
    } 
    else if (arg->type == ID_TYPE) {
        if (arg->index == -1) {return 0;}
        //ERASE
        cout << "Argument resolved to " << mem[arg->index] << endl;
        return mem[arg->index];  // Retrieve value from value list
    } 
    else if (arg->type == POLYEVAL_TYPE) {
        return resolve_polynomial(arg->poly_eval, poly_section);  // Recursive resolution
    } 
    else {
        cerr << "Unknown argument type in resolve_argument\n";
        return -1;
    }
}
int resolve_term(TERM* term, const vector<int>& arg_vals){
    if (!term){
        cerr << "Null term or monom list declaration in resolve term\n";
        return -1;
    }
    int termValue = 0;
    /*STEP 1 resolve monom list*/
    if (term->monomial_list && term->monomial_list->head){
        MONOMIAL* monom = term->monomial_list->head;
        termValue = resolve_monomial(monom, arg_vals);
        termValue *= term->coefficient;
    } else {
        termValue = term->coefficient;
    }
    if (term->next){
        int val2 = resolve_term(term->next, arg_vals);
        if (term->addop == PLUS_SIGN){
            termValue += val2;
        }else if (term->addop == MINUS_SIGN){
            termValue -= val2;
        }
    }
    //cout << "Term value: " << termValue << endl;
    return termValue;
}
int resolve_monomial(MONOMIAL* monom, const vector<int>& arg_vals){
    if (!monom){return 0;}
    //cout << "Resolving monomial \n";
        /*STEP 1 resolve primary */
        int primaryValue = resolve_primary(monom->primary, arg_vals);
        //cout << "Value at resolve monom is " << primaryValue << "\n";
        /*STEP 2 resolve exponent application*/
        int monoVal = pow(primaryValue, monom->exponent);
        /*STEP 3 recursive monomial execution*/
        if (monom->next){
            monoVal *= resolve_monomial(monom->next, arg_vals);
        }
        cout << "Monomial resolved to " << monoVal << endl;
        return monoVal;
}
int resolve_primary(PRIMARY* primary, const vector<int>& arg_vals){
    //cout << "Resolving primary\n";
    /*STEP 1 identify type of primary ID or term_list */
    if (primary->type == IDENFIER){
        cout << primary->lexeme << " resolved to " << mem[get_varmap(primary->lexeme)] << endl;
        return mem[get_varmap(primary->lexeme)];
    }else if (primary->type == RTERML){
        return resolve_term(primary->term_list->head, arg_vals);
    }
    cerr << "Problem, returning -1";
    return -1;
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
   unordered_set<string> polyNames; // will hold poly decl
   for (const POLY_DECL& polyDecl : *program->poly_section) {
    polyNames.insert(polyDecl.name);
    }   

    //iterate through statements in execute section
    STATEMENT* currentStmt = program->execute_section->head;
    while (currentStmt) {
        //if cur stmt has a poly eval, check it 
        if (currentStmt->poly_evaluation_t) {
            check_poly_eval(currentStmt->poly_evaluation_t, polyNames, error_lines);
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
void check_poly_eval(POLY_EVAL* poly_eval, unordered_set<string> polyNames, vector<int>& error_lines){
    if (!poly_eval) return;
    // Check if the polynomial evaluation name exists in declarations.
    if (polyNames.find(poly_eval->name) == polyNames.end()) {
        error_lines.push_back(poly_eval->line_no);
    } 
    //recursive check
    if (poly_eval->argument_list){check_arg_list(poly_eval->argument_list, polyNames, error_lines);
    return;
    }
    return;
}
void check_arg_list(ARGUMENT_LIST* argument_list, unordered_set<string> polyNames, vector<int>& error_lines){
    if (!argument_list || !argument_list->head) {
        return;
    }
    ARGUMENT* argument = argument_list->head;
    while (argument){
        if(argument->type == NUM_TYPE || argument->type == ID_TYPE){
            //skip num and idenfiters
            argument = argument->next;
            continue;
        }
        else{
            check_poly_eval(argument->poly_eval, polyNames, error_lines);
        }
        argument = argument->next;
    }
    return;
}
bool code_4(PROGRAM* program){
    vector<int> error_lines; 
    if (!program->execute_section || !program->execute_section->head){
        cerr << "Null pointer found in code 4\n";
        return false;
    }
    STATEMENT* statement = program->execute_section->head;

    while(statement){
        if (statement->statement_type == ASSIGN_STATEMENT){
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
    int expected_args = poly_decl->poly_parameters->size();
    int eval_args = 0;
    ARGUMENT* argument = poly_eval->argument_list->head;
   //cout << "Argument pointer made\n";
    while (argument) {
        eval_args++; //collect argument count
        if (argument->type == POLYEVAL_TYPE){
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
//var map get and set  
int grow_varmap(string lexeme){
    auto it = find(var_map.begin(), var_map.end(), lexeme);
    if (it != var_map.end()){
        return -1;
    }
    var_map.push_back(lexeme);
    mem[var_index] = 0; //initialize to 0
    return var_index++;
}
int get_varmap(string lexeme){
    for (size_t i = 0; i < var_map.size(); i++) { 
        if (var_map[i] == lexeme) { 
            return i; // Return index if found
        }
    }
    return -1; // Return -1 if not found
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
            printf("STATEMENT %zu: Type %d, Var %d\n", 
                i, current->statement_type, current->var);
            current = current->next; // Move to the next statement
            i++;
            }
        } 
    else {
        cout << "No EXECUTE section.\n";
    }
}