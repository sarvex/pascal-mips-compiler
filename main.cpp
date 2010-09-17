#include "parser.h"

bool check_semantics();

int main() {
    Program * program = parse_input();
    
    if (! check_semantics())
        return -1;

    return 0;
}

// check everything that could possibly go wrong. 
// return true if everything is OK
bool check_semantics(program) {
    // one pass to collect data

    //



    // for each class
    ClassList * class_list = program->class_list;
    while (class_list != NULL) {
        // make sure parent class exists
        class_list = class_list->next;
    }
}
