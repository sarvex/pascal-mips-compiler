#include "symbol_table.h"

SymbolTable * build_symbol_table(Program * program) {
    SymbolTable * symbol_table = new SymbolTable();

    // collect all the classes that are declared
    for (ClassList * class_list = program->class_list; class_list != NULL; class_list = class_list->next) {
        ClassDeclaration * class_declaration = class_list->item;

        // add the class to symbol table
        (*symbol_table)[class_declaration->id] = new ClassSymbolTable(class_declaration);

        // add each class variable to symbol table
        std::map<std::string, VariableDeclaration *> * variables = (*symbol_table)[class_declaration->id]->variables;
        for (VariableDeclarationList * variable_list = class_declaration->class_block->variable_list; variable_list != NULL; variable_list = variable_list->next) {
            VariableDeclaration * variable_declaration = variable_list->item;
            for (IdentifierList * id_list = variable_declaration->id_list; id_list != NULL; id_list = id_list->next) {
                (*variables)[id_list->item] = variable_declaration;
            }
        }

        // for each function
        std::map<std::string, FunctionSymbolTable *> * function_symbols = (*symbol_table)[class_declaration->id]->function_symbols;
        for (FunctionDeclarationList * function_list = class_declaration->class_block->function_list; function_list != NULL; function_list = function_list->next) {
            FunctionDeclaration * function_declaration = function_list->item;

            // add the function to symbol table
            (*function_symbols)[function_declaration->id] = new FunctionSymbolTable(function_declaration);
            std::map<std::string, VariableDeclaration *> * function_variables = (*function_symbols)[function_declaration->id]->variables;

            // add function variables to symbol table
            for (VariableDeclarationList * variable_list = function_declaration->block->variable_list; variable_list != NULL; variable_list = variable_list->next) {
                VariableDeclaration * variable_declaration = variable_list->item;
                for (IdentifierList * id_list = variable_declaration->id_list; id_list != NULL; id_list = id_list->next) {
                    (*function_variables)[id_list->item] = variable_declaration;
                }
            }
        }
    }

    return symbol_table;
}
