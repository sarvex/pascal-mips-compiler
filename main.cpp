
#include "parser.h"
#include "symbol_table.h"
#include "semantic_checker.h"
#include "code_generation.h"

#include <string>

void print_usage(std::string exe_name);

int main(int argc, char * argv[]) {
    char * filename = NULL;

    bool print_assembly = false;
    for (int i=1; i<argc; ++i) {
        std::string arg = argv[i];
        if (arg[0] == '-') {
            if (arg.compare("-S") == 0) {
                print_assembly = true;
            } else {
                std::cerr << "Unrecognized parameter: " << arg << std::endl;
                print_usage(argv[0]);
                return 1;
            }
        } else {
            filename = argv[i];
        }
    }

    Program * program = parse_input(filename);

    SymbolTable * symbol_table = build_symbol_table(program);
    if (symbol_table == NULL)
        return 1;

    bool semantic_success = SemanticChecker::check(program, symbol_table);
    if (!semantic_success)
        return 1;

    if (print_assembly) {
        generate_code(program);
    }

    return 0;
}

void print_usage(std::string exe_name) {
    std::cerr << "Usage: \n\n";
    std::cerr << "Compile a file:\n\n";
    std::cerr << exe_name << " [file]\n\n";
    std::cerr << "Show assembly output:\n";
    std::cerr << exe_name << " -s [file]\n";
}
