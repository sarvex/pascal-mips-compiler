#include "parser.h"

#include <map>
#include <iostream>
using std::cout;
using std::endl;
using std::map;

bool check_semantics(Program *);

int main() {
    Program * program = parse_input();
    
    if (! check_semantics(program))
        return -1;

    return 0;
}

// check everything that could possibly go wrong. 
// return true if everything is OK
bool check_semantics(Program * program) {
    // one pass to collect data

    // collect all the classes that are declared
    map<char *, ClassDeclaration *> class_map;
    ClassList * class_list = program->class_list;
    while (class_list != NULL) {
        ClassDeclaration * class_declaration = class_list->item;
        class_map[class_declaration->id] = class_declaration;
        cout << "Found class: " << class_declaration->id << endl;
        class_list = class_list->next;
    }


    // now pass to validate
    class_list = program->class_list;
    while (class_list != NULL) {
        // make sure parent class exists
        // TODO


        class_list = class_list->next;
    }

    return true;
}
