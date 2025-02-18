#include <vector>
#include <string>
#include <iostream>
#include "resolution.h"
#include "parser.h"

using namespace std;

/*
Global var decl
*/
static vector<pair<string, int> > var_map;
static int var_index = 0;
static vector<pair<int, int> > value_list;
int syntax_execution = 0;

//SEMANTIC CHECKING 
void task_execution(PROGRAM* program){
    //traverse the task list and execute 
    if(program->tasks){
        for (int task : *program->tasks){
            switch (task){
                case 1:
                    task_1(program);
                break;
            case 2:
                //handleTask2();
                break;
            case 3:
                //handleTask3();
                break;
            case 4:
                //handleTask4();
                break;
            default:
                std::cout << "Unknown task: " << task << "\n";

            }
        }
    }else {
        cerr << "Tasks is found as nullptr in task execution";
    }
}

void task_1(PROGRAM* program){
    //syntax handling
    int status = syntax_execution_get(); 
    if (status > 0) {
        cout << "SYNTAX ERROR !!!!!&%!!\n";
    }
    /*
    SEMANTIC HANDLING 
    */
   if (code_1(program->poly_section)) return;
   if (code_2(program->poly_section)) return;
   if (code_3(program)) return;
   if (code_4(program)) return;   
   else {
    cout << "No semantic errors found\n";
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
    //case no args
    for (const auto& poly : *poly_section){
        if (poly.poly_parameters == nullptr){
            if (poly.body){
                invalid_line = process_term_list_univariate(poly.body, invalid_line);
            }
        } else {
            vector<string> poly_vars;
            for (const auto& param : *poly.poly_parameters) {
                poly_vars.push_back(param.first); // Collect variable names (like x, y, etc.)
            }

            if (poly.body) {
                invalid_line = process_term_list_multivariate(poly.body, invalid_line, poly_vars);
            }
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
vector<int> process_term_list_univariate(TERM_LIST* term_list, vector<int> invalid_line){
     /*
    for every term in term list
        for every monomial in monom list of term
            if primary is of type identifier,it should be an index where x is in var map 
            if is of type term list, for every term in its term list 
                for every monomial in monom list of term
                    if primary is of type identifier,it should be an index where x is in var map 
                    if is of type term list, for every term in its term list etc.. 
            */
TERM* term = term_list ? term_list->head : nullptr;
while (term){
    MONOMIAL* monomial = term->monomial_list ? term->monomial_list->head : nullptr;
        while (monomial){
            if (monomial->primary->type == IDENFIER){
                int var_map_index = get_varmap("x");
                if (var_map_index == -1) {
                    cerr << "Error: Identifier not found in var_map\n";
                    invalid_line.push_back(monomial->primary->line_no);
                }
                else if (var_map_index != monomial->primary->var_index){
                    invalid_line.push_back(monomial->primary->line_no);
                }
            } 
            else if (monomial->primary->type == RTERML){
                invalid_line = process_term_list_univariate(monomial->primary->term_list, invalid_line);
            }
            monomial = monomial->next;
        }
        term = term->next;
    }
    return invalid_line;
}
vector<int> process_term_list_multivariate(TERM_LIST* term_list, vector<int> invalid_line, const vector<string>& poly_vars) {
    TERM* term = term_list ? term_list->head : nullptr;
    while (term) {
        MONOMIAL* monomial = term->monomial_list ? term->monomial_list->head : nullptr;
        while (monomial) {
            if (monomial->primary->type == IDENFIER) {
                // Check if the identifier is part of the valid polynomial variables
                for (const string& var : poly_vars) {
                    if (monomial->primary->var_index == get_varmap(var)) {
                        break;
                    }
                    else {
                        // Invalid monomial name found, add the line number to the invalid list
                        invalid_line.push_back(monomial->primary->line_no);
                    }
                }
            } else if (monomial->primary->type == RTERML) {
                // Recursively process nested term lists
                invalid_line = process_term_list_multivariate(monomial->primary->term_list, invalid_line, poly_vars);
            }
            monomial = monomial->next; // Move to the next monomial
        }
        term = term->next; // Move to the next term
    }
    return invalid_line; // Return the list of invalid lines
}
bool code_3(PROGRAM* program){
    return false;
}
bool code_4(PROGRAM* program){
    return false;
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
    var_index++;
    return 0;
}

int get_varmap(string lexeme){
    for (const auto& entry: var_map){
        if (entry.first == lexeme){
            return entry.second;
        }
    }
    cerr << "Error: Variable '" << lexeme << "' not found!\n";
    return -1; // Error value
}

int get_valuelist(int index, int value){
    return 0;   
}
int set_valuelist(int index, int value){
    return 0;   
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