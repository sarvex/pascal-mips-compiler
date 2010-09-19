#include "symbol_table.h"

#include "utils.h"
using Utils::err_header;

#include <cassert>

SymbolTable * build_symbol_table(Program * program) {
    SymbolTable * symbol_table = new SymbolTable();
    bool success = true;

    // collect all the classes that are declared
    for (ClassList * class_list = program->class_list; class_list != NULL; class_list = class_list->next) {
        ClassDeclaration * class_declaration = class_list->item;

        // add the class to symbol table
        if (symbol_table->has_key(class_declaration->identifier->text)) {
            ClassDeclaration * other_class = symbol_table->item(class_declaration->identifier->text)->class_declaration;
            std::cerr << err_header(class_declaration->identifier->line_number) <<
                "class \"" << other_class->identifier->text << "\" already declared at line " <<
                other_class->identifier->line_number << std::endl;
            success = false;
            continue;
        } else {
            symbol_table->put(class_declaration->identifier->text, new ClassSymbolTable(class_declaration));
        }

        // add each class variable to symbol table
        InsensitiveMap<VariableData *> * variables = symbol_table->item(class_declaration->identifier->text)->variables;
        for (VariableDeclarationList * variable_list = class_declaration->class_block->variable_list; variable_list != NULL; variable_list = variable_list->next) {
            VariableDeclaration * variable_declaration = variable_list->item;
            for (IdentifierList * id_list = variable_declaration->id_list; id_list != NULL; id_list = id_list->next) {
                if (variables->has_key(id_list->item->text)) {
                    VariableData * other_variable = variables->item(id_list->item->text);
                    std::cerr << err_header(id_list->item->line_number) << "variable \"" <<
                        id_list->item->text << "\" already declared at line " <<
                        other_variable->line_number << std::endl;
                    success = false;
                } else {
                    variables->put(id_list->item->text, new VariableData(variable_declaration->type, id_list->item->line_number));
                }
            }
        }

        // for each function
        InsensitiveMap<FunctionSymbolTable *> * function_symbols = symbol_table->item(class_declaration->identifier->text)->function_symbols;
        for (FunctionDeclarationList * function_list = class_declaration->class_block->function_list; function_list != NULL; function_list = function_list->next) {
            FunctionDeclaration * function_declaration = function_list->item;

            // add the function to symbol table
            if (function_symbols->has_key(function_declaration->identifier->text)) {
                std::cerr << err_header(function_declaration->identifier->line_number) <<
                    "function \"" << function_declaration->identifier->text << "\" already declared at line " <<
                    function_symbols->item(function_declaration->identifier->text)->function_declaration->identifier->line_number << std::endl;
                success = false;
                continue;
            }
            function_symbols->put(function_declaration->identifier->text, new FunctionSymbolTable(function_declaration));
            InsensitiveMap<VariableData *> * function_variables = function_symbols->item(function_declaration->identifier->text)->variables;

            // add the function name to function symbol table
            function_variables->put(function_declaration->identifier->text,
                new VariableData(function_declaration->type, function_declaration->identifier->line_number));

            // add function parameters to symbol table
            for (VariableDeclarationList * parameter_list = function_declaration->parameter_list; parameter_list != NULL; parameter_list = parameter_list->next)
                success &= add_variables(function_variables, parameter_list->item, function_declaration->identifier->text);

            // add function variables to symbol table
            for (VariableDeclarationList * variable_list = function_declaration->block->variable_list; variable_list != NULL; variable_list = variable_list->next)
                success &= add_variables(function_variables, variable_list->item, function_declaration->identifier->text);

        }
    }

    // check for duplicate declared variables because of inheritance
    for (ClassList * class_list = program->class_list; class_list != NULL; class_list = class_list->next) {
        ClassDeclaration * class_declaration = class_list->item;
        if (class_declaration->parent_identifier == NULL)
            continue;
        for (VariableDeclarationList * variable_list = class_declaration->class_block->variable_list; variable_list != NULL; variable_list = variable_list->next) {
            VariableDeclaration * variable_declaration = variable_list->item;
            for (IdentifierList * id_list = variable_declaration->id_list; id_list != NULL; id_list = id_list->next) {
                VariableData * other_field = get_field(symbol_table, class_declaration->parent_identifier->text, id_list->item->text);
                if (other_field != NULL) {
                    std::cerr << err_header(id_list->item->line_number) << "variable \"" <<
                        id_list->item->text << "\" already declared at line " <<
                        other_field->line_number << std::endl;
                    success = false;
                }
            }
        }

    }

    return success ? symbol_table : NULL;
}

VariableData * get_field(SymbolTable * symbol_table, std::string class_name, std::string field_name) {
    ClassSymbolTable * class_symbols = symbol_table->item(class_name);
    if (class_symbols->variables->has_key(field_name)) {
        return class_symbols->variables->item(field_name);
    } else if (class_symbols->class_declaration->parent_identifier == NULL) {
        return NULL;
    } else {
        return get_field(symbol_table, class_symbols->class_declaration->parent_identifier->text, field_name);
    }
}

bool add_variables(InsensitiveMap<VariableData *> * function_variables, VariableDeclaration * variable_declaration, std::string function_name) {
    bool success = true;
    for (IdentifierList * id_list = variable_declaration->id_list; id_list != NULL; id_list = id_list->next) {
        if (! function_variables->has_key(id_list->item->text)) {
            function_variables->put(id_list->item->text, new VariableData(variable_declaration->type, id_list->item->line_number));
        } else {
            if (function_name.compare(id_list->item->text) == 0) {
                std::cerr << err_header(id_list->item->line_number) <<
                    "variable name \"" << id_list->item->text << "\" is reserved for use as the function return value" << std::endl;
            } else {
                std::cerr << err_header(id_list->item->line_number) <<
                    "variable \"" << id_list->item->text << "\" already declared at line " <<
                    function_variables->item(id_list->item->text)->line_number << std::endl;
            }
            success = false;
        }
    }
    return success;
}

