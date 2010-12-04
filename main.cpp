
#include "parser.h"
#include "symbol_table.h"
#include "semantic_checker.h"
#include "code_generation.h"

#include <string>

void print_usage(std::string exe_name);

int main(int argc, char * argv[]) {
    char * filename = NULL;

    bool only_semantic_checking = false;
    bool output_intermediate = false;
    bool disable_optimization = false;
    for (int i=1; i<argc; ++i) {
        std::string arg = argv[i];
        if (arg[0] == '-') {
            if (arg.compare("-p1") == 0) {
                only_semantic_checking = true;
            } else if (arg.compare("-p2") == 0) {
                output_intermediate = true;
            } else if (arg.compare("-O0") == 0) {
                disable_optimization = true;
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
    if (only_semantic_checking)
        return 0;

    generate_code(program, output_intermediate, disable_optimization);

    return 0;
}

void print_usage(std::string exe_name) {
    std::cerr << "Usage: \n\n";

    std::cerr << "Compile a file into MIPS assembly:\n\n";
    std::cerr << exe_name << " [file]\n\n";

    std::cerr << "Stop after semantic checking:\n";
    std::cerr << exe_name << " -p1 [file]\n";

    std::cerr << "Output optimized intermediate representation:\n";
    std::cerr << exe_name << " -p2 [file]\n";

    std::cerr << "Disable optimization:\n";
    std::cerr << exe_name << " -O0 [file]\n";
}
