
#include "parser.h"
#include "symbol_table.h"
#include "semantic_checker.h"

int main(int argc, char ** argv) {
    char * filename = NULL;
    if (1 < argc) {
        filename = argv[1];
    }

    Program * program = parse_input(filename);
    SymbolTable * symbol_table = build_symbol_table(program);
    if (symbol_table == NULL)
        return 1;

    bool semantic_success = SemanticChecker::check(program, symbol_table);
    if (!semantic_success)
        return 1;

    return 0;
}
