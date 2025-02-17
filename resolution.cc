#include <vector>
#include <string>
#include <iostream>

#include "parser.h"

using namespace std;

static vector<pair<string, int>> var_map;
static vector<pair<int, int>> value_list;

int grow_varmap(string lexeme){
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