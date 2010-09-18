#include <cassert>

#include "symbol_table.h"
#include "utils.h"

SymbolTable * build_symbol_table(Program * program) {
    SymbolTable * symbol_table = new SymbolTable();
    bool success = true;

    // collect all the classes that are declared
    for (ClassList * class_list = program->class_list; class_list != NULL; class_list = class_list->next) {
        ClassDeclaration * class_declaration = class_list->item;

        // add the class to symbol table
        (*symbol_table)[class_declaration->identifier->text] = new ClassSymbolTable(class_declaration);

        // add each class variable to symbol table
        std::map<std::string, VariableDeclaration *> * variables = (*symbol_table)[class_declaration->identifier->text]->variables;
        for (VariableDeclarationList * variable_list = class_declaration->class_block->variable_list; variable_list != NULL; variable_list = variable_list->next) {
            VariableDeclaration * variable_declaration = variable_list->item;
            for (IdentifierList * id_list = variable_declaration->id_list; id_list != NULL; id_list = id_list->next) {
                (*variables)[id_list->item->text] = variable_declaration;
            }
        }

        // for each function
        std::map<std::string, FunctionSymbolTable *> * function_symbols = (*symbol_table)[class_declaration->identifier->text]->function_symbols;
        for (FunctionDeclarationList * function_list = class_declaration->class_block->function_list; function_list != NULL; function_list = function_list->next) {
            FunctionDeclaration * function_declaration = function_list->item;

            // add the function to symbol table
            (*function_symbols)[function_declaration->identifier->text] = new FunctionSymbolTable(function_declaration);
            std::map<std::string, VariableDeclaration *> * function_variables = (*function_symbols)[function_declaration->identifier->text]->variables;

            // add function variables to symbol table
            for (VariableDeclarationList * variable_list = function_declaration->block->variable_list; variable_list != NULL; variable_list = variable_list->next) {
                VariableDeclaration * variable_declaration = variable_list->item;
                for (IdentifierList * id_list = variable_declaration->id_list; id_list != NULL; id_list = id_list->next) {
                    if (function_variables->count(id_list->item->text) == 0) {
                        (*function_variables)[id_list->item->text] = variable_declaration;
                    } else {
                        std::cerr << Utils::err_header(id_list->item->line_number) << "variable \"" << id_list->item->text << "\" already declared at line ";
                        // figure out which line the previous declaration was on
                        VariableDeclaration * other_declaration = (*function_variables)[id_list->item->text];
                        for (IdentifierList * other_id_list = other_declaration->id_list; other_id_list != NULL; other_id_list = other_id_list->next) {
                            if (other_id_list->item->text != id_list->item->text)
                                continue;
                            // there it is. print the line number.
                            std::cerr << other_id_list->item->line_number << std::endl;
                            success = false;
                            goto continue_id_loop;
                        }
                        assert(false);
                    }
                    continue_id_loop:;
                }
            }

            // add function parameters to symbol table
            for (VariableDeclarationList * parameter_list = function_declaration->parameter_list; parameter_list != NULL; parameter_list = parameter_list->next) {
                VariableDeclaration * parameter = parameter_list->item;
                for (IdentifierList * id_list = parameter->id_list; id_list != NULL; id_list = id_list->next) {
                    (*function_variables)[id_list->item->text] = parameter;
                }
            }
        }
    }

    return success ? symbol_table : NULL;
}
