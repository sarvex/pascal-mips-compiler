
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
    SemanticChecker * checker = new SemanticChecker(program, symbol_table);

    if (! checker->check())
        return -1;

    return 0;
}
