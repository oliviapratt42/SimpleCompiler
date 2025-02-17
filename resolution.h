#ifndef resolution.h
#define resolution.h

#include <vector>
#include <string>
#include "parser.h"

int grow_varmap(string lexeme);
int get_varmap(string lexeme);
int get_valuelist(int index, int value);
int set_valuelist(int index, int value);
#endif