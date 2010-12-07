#include "symbol_table.h"

#include "utils.h"
using Utils::err_header;

#include <cassert>

SymbolTable * build_symbol_table(Program * program) {
    SymbolTable * symbol_table = new SymbolTable();
    bool success = true;

    // reverse all VariableDeclarationLists
    for (ClassList * class_list = program->class_list; class_list != NULL; class_list = class_list->next) {
        ClassDeclaration * class_declaration = class_list->item;
        class_declaration->class_block->variable_list = reverse_variable_declaration_list(class_declaration->class_block->variable_list);

        for (FunctionDeclarationList * function_list = class_declaration->class_block->function_list; function_list != NULL; function_list = function_list->next) {
            FunctionDeclaration * function_declaration = function_list->item;

            function_declaration->parameter_list = reverse_variable_declaration_list(function_declaration->parameter_list);
            function_declaration->block->variable_list = reverse_variable_declaration_list(function_declaration->block->variable_list);
        }
    }

    // collect all the classes that are declared
    for (ClassList * class_list = program->class_list; class_list != NULL; class_list = class_list->next) {
        ClassDeclaration * class_declaration = class_list->item;

        // add the class to symbol table
        if (symbol_table->has_key(class_declaration->identifier->text)) {
            ClassDeclaration * other_class = symbol_table->get(class_declaration->identifier->text)->class_declaration;
            std::cerr << err_header(class_declaration->identifier->line_number) <<
                "class \"" << other_class->identifier->text << "\" already declared at line " <<
                other_class->identifier->line_number << std::endl;
            success = false;
            continue;
        } else {
            symbol_table->put(class_declaration->identifier->text, new ClassSymbolTable(class_declaration));
        }

        // add each class variable to symbol table
        OrderedInsensitiveMap<VariableData *> * variables = symbol_table->get(class_declaration->identifier->text)->variables;
        for (VariableDeclarationList * variable_list = class_declaration->class_block->variable_list; variable_list != NULL; variable_list = variable_list->next) {
            VariableDeclaration * variable_declaration = variable_list->item;
            for (IdentifierList * id_list = variable_declaration->id_list; id_list != NULL; id_list = id_list->next) {
                if (variables->has_key(id_list->item->text)) {
                    VariableData * other_variable = variables->get(id_list->item->text);
                    std::cerr << err_header(id_list->item->line_number) << "variable \"" <<
                        id_list->item->text << "\" already declared at line " <<
                        other_variable->line_number << std::endl;
                    success = false;
                } else {
                    variables->put(id_list->item->text, new VariableData(variable_declaration->type, id_list->item->line_number, id_list->item->text));
                }
            }
        }

        // for each function
        OrderedInsensitiveMap<FunctionSymbolTable *> * function_symbols = symbol_table->get(class_declaration->identifier->text)->function_symbols;
        for (FunctionDeclarationList * function_list = class_declaration->class_block->function_list; function_list != NULL; function_list = function_list->next) {
            FunctionDeclaration * function_declaration = function_list->item;

            // add the function to symbol table
            if (function_symbols->has_key(function_declaration->identifier->text)) {
                std::cerr << err_header(function_declaration->identifier->line_number) <<
                    "function \"" << function_declaration->identifier->text << "\" already declared at line " <<
                    function_symbols->get(function_declaration->identifier->text)->function_declaration->identifier->line_number << std::endl;
                success = false;
                continue;
            }
            function_symbols->put(function_declaration->identifier->text, new FunctionSymbolTable(function_declaration));
            OrderedInsensitiveMap<VariableData *> * function_variables = function_symbols->get(function_declaration->identifier->text)->variables;

            // add the function name to function symbol table
            function_variables->put(function_declaration->identifier->text,
                new VariableData(function_declaration->type, function_declaration->identifier->line_number, function_declaration->identifier->text));

            // add function parameters to symbol table
            for (VariableDeclarationList * parameter_list = function_declaration->parameter_list; parameter_list != NULL; parameter_list = parameter_list->next)
                success &= add_variables(function_variables, parameter_list->item, function_declaration->identifier->text);

            // add function variables to symbol table
            for (VariableDeclarationList * variable_list = function_declaration->block->variable_list; variable_list != NULL; variable_list = variable_list->next)
                success &= add_variables(function_variables, variable_list->item, function_declaration->identifier->text);

        }
    }

    // check for duplicate declared variables and methods because of inheritance
    for (ClassList * class_list = program->class_list; class_list != NULL; class_list = class_list->next) {
        ClassDeclaration * class_declaration = class_list->item;
        if (class_declaration->parent_identifier == NULL)
            continue;

        // check for inheritance loop
        if (inheritance_loop(symbol_table, class_declaration->identifier->text)) {
            std::cerr << err_header(class_declaration->identifier->line_number) <<
                "inheritance loop detected" << std::endl;
            success = false;
            continue;
        }

        // make sure parent class is defined
        if (! symbol_table->has_key(class_declaration->parent_identifier->text)) {
            std::cerr << err_header(class_declaration->identifier->line_number) << "class \"" <<
                class_declaration->identifier->text << "\" attempted to extend class \"" <<
                class_declaration->parent_identifier->text << "\" which does not exist" << std::endl;
            success = false;
            continue;
        }

        // variables
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

        // methods
        for (FunctionDeclarationList * function_list = class_declaration->class_block->function_list; function_list != NULL; function_list = function_list->next) {
            FunctionDeclaration * function_declaration = function_list->item;
            FunctionDeclaration * other_function = get_method(symbol_table, class_declaration->parent_identifier->text, function_declaration->identifier->text);
            if (other_function != NULL) {
                std::cerr << err_header(function_declaration->identifier->line_number) << "method \"" <<
                    other_function->identifier->text << "\" already declared at line " <<
                    other_function->identifier->line_number << std::endl;
                success = false;
            }
        }
    }

    return success ? symbol_table : NULL;
}

bool inheritance_loop(SymbolTable * symbol_table, std::string class_name) {
    ClassSymbolTable * class_symbols = symbol_table->get(class_name);
    return inheritance_loop(symbol_table, class_name, class_symbols->class_declaration->parent_identifier->text);
}

bool inheritance_loop(SymbolTable * symbol_table, std::string original_class, std::string current_class) {
    if (! symbol_table->has_key(current_class))
        return false;
    if (Utils::insensitive_equals(original_class, current_class))
        return true;
    ClassSymbolTable * class_symbols = symbol_table->get(current_class);
    if (class_symbols->class_declaration->parent_identifier == NULL)
        return false;
    return inheritance_loop(symbol_table, original_class, class_symbols->class_declaration->parent_identifier->text);
}

FunctionDeclaration * get_method(SymbolTable * symbol_table, std::string class_name, std::string method_name) {
    if (! symbol_table->has_key(class_name))
        return NULL;
    ClassSymbolTable * class_symbols = symbol_table->get(class_name);
    if (class_symbols->function_symbols->has_key(method_name))
        return class_symbols->function_symbols->get(method_name)->function_declaration;
    if (class_symbols->class_declaration->parent_identifier == NULL)
        return NULL;
    return get_method(symbol_table, class_symbols->class_declaration->parent_identifier->text, method_name);
}

std::string get_declaring_class(SymbolTable * symbol_table, std::string class_name, std::string method_name) {
    assert(symbol_table->has_key(class_name));

    ClassSymbolTable * class_symbols = symbol_table->get(class_name);
    if (class_symbols->function_symbols->has_key(method_name))
        return class_name;

    assert(class_symbols->class_declaration->parent_identifier != NULL);

    return get_declaring_class(symbol_table, class_symbols->class_declaration->parent_identifier->text, method_name);
}


VariableData * get_field(SymbolTable * symbol_table, std::string class_name, std::string field_name) {
    if (! symbol_table->has_key(class_name))
        return NULL;
    ClassSymbolTable * class_symbols = symbol_table->get(class_name);
    if (class_symbols->variables->has_key(field_name)) {
        return class_symbols->variables->get(field_name);
    } else if (class_symbols->class_declaration->parent_identifier == NULL) {
        return NULL;
    } else {
        return get_field(symbol_table, class_symbols->class_declaration->parent_identifier->text, field_name);
    }
}

bool add_variables(OrderedInsensitiveMap<VariableData *> * function_variables, VariableDeclaration * variable_declaration, std::string function_name) {
    bool success = true;
    for (IdentifierList * id_list = variable_declaration->id_list; id_list != NULL; id_list = id_list->next) {
        if (! function_variables->has_key(id_list->item->text)) {
            function_variables->put(id_list->item->text, new VariableData(variable_declaration->type, id_list->item->line_number, id_list->item->text));
        } else {
            if (function_name.compare(id_list->item->text) == 0) {
                std::cerr << err_header(id_list->item->line_number) <<
                    "variable name \"" << id_list->item->text << "\" is reserved for use as the function return value" << std::endl;
            } else {
                std::cerr << err_header(id_list->item->line_number) <<
                    "variable \"" << id_list->item->text << "\" already declared at line " <<
                    function_variables->get(id_list->item->text)->line_number << std::endl;
            }
            success = false;
        }
    }
    return success;
}

VariableDeclarationList * reverse_variable_declaration_list(VariableDeclarationList * variable_declaration_list, VariableDeclarationList * prev)
{
    if (variable_declaration_list == NULL)
        return NULL;
    VariableDeclarationList * ret;
    if (variable_declaration_list->next != NULL)
        ret = reverse_variable_declaration_list(variable_declaration_list->next, variable_declaration_list);
    else
        ret = variable_declaration_list;
    variable_declaration_list->next = prev;
    return ret;
}

