#include "parser.h"
#include "symbol_table.h"
#include "semantic_checker.h"
#include "code_generation.h"

#include <string>

void print_usage(std::string exe_name);
void add_entry_point(Program * program);

int main(int argc, char * argv[]) {
    char * filename = NULL;

    bool only_semantic_checking = false;
    bool output_intermediate = false;
    bool disable_optimization = false;
    bool skip_lame_stuff = false;
    for (int i=1; i<argc; ++i) {
        std::string arg = argv[i];
        if (arg[0] == '-') {
            if (arg.compare("-p1") == 0) {
                only_semantic_checking = true;
            } else if (arg.compare("-p2") == 0) {
                output_intermediate = true;
            } else if (arg.compare("-O0") == 0) {
                disable_optimization = true;
            } else if (arg.compare("-s") == 0) {
                skip_lame_stuff = true;
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

    add_entry_point(program);


    SymbolTable * symbol_table = build_symbol_table(program);
    if (symbol_table == NULL)
        return 1;

    bool semantic_success = SemanticChecker::check(program, symbol_table);
    if (!semantic_success)
        return 1;
    if (only_semantic_checking)
        return 0;

    generate_code(program, symbol_table, output_intermediate, disable_optimization, skip_lame_stuff);

    return 0;
}

void add_entry_point(Program * program) {
    VariableDeclarationList * main_instance = new VariableDeclarationList(new VariableDeclaration(new IdentifierList(new Identifier("_instance", -1), NULL), new TypeDenoter(program->identifier)), NULL);
    FunctionDeclaration * main_function = new FunctionDeclaration(new Identifier("_entrypoint", -1), NULL, NULL, new FunctionBlock(main_instance, new StatementList(
        new Statement(new AssignmentStatement(new VariableAccess(new Identifier("_instance", -1)),
        new Expression(new AdditiveExpression(new MultiplicativeExpression(new NegatableExpression(
        new PrimaryExpression(new ObjectInstantiation(program->identifier)))))))), NULL)));
    ClassDeclaration * class_declaration = new ClassDeclaration(new Identifier("_entrypoint", -1), NULL,
        new ClassBlock(NULL, new FunctionDeclarationList(main_function, NULL)));

    program->class_list = new ClassList(class_declaration, program->class_list);
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
