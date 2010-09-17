
#include "parser.h"

Program * new_program(char* id, struct class_list_t * class_list) {
    Program * program = NEW(Program);
    program->id = id;
    program->class_list = class_list;
    return program;
}

