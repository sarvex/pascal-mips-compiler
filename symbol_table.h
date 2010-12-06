#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "parser.h"

#include "insensitive_map.h"
#include <string>


struct VariableData {
    TypeDenoter * type;
    int line_number;
    std::string name;

    VariableData(TypeDenoter * type, int line_number, std::string name) :
        type(type),
        line_number(line_number),
        name(name) {}
};
typedef OrderedInsensitiveMap<VariableData *> VariableTable;

struct FunctionSymbolTable {
    FunctionDeclaration * function_declaration;
    // function variables, maps variable name to variable declaration
    VariableTable * variables;

    FunctionSymbolTable(FunctionDeclaration * function_declaration) :
        function_declaration(function_declaration),
        variables(new OrderedInsensitiveMap<VariableData *>) {}
};

struct ClassSymbolTable {
    ClassDeclaration * class_declaration;
    // class variables, maps variable name to variable declaration
    VariableTable * variables;
    // maps function name to function symbol table
    OrderedInsensitiveMap<FunctionSymbolTable *> * function_symbols;

    ClassSymbolTable(ClassDeclaration * class_declaration) :
        class_declaration(class_declaration),
        variables(new OrderedInsensitiveMap<VariableData *>),
        function_symbols(new OrderedInsensitiveMap<FunctionSymbolTable *>) {}
};
// maps class name to symbol table
typedef OrderedInsensitiveMap<ClassSymbolTable *> SymbolTable;

SymbolTable * build_symbol_table(Program * program);



bool add_variables(OrderedInsensitiveMap<VariableData *> * function_variables, VariableDeclaration * variable_declaration, std::string function_name);
VariableData * get_field(SymbolTable * symbol_table, std::string class_name, std::string field_name);
FunctionDeclaration * get_method(SymbolTable * symbol_table, std::string class_name, std::string method_name);
std::string get_declaring_class(SymbolTable * symbol_table, std::string class_name, std::string method_name);
VariableDeclarationList * reverse_variable_declaration_list(VariableDeclarationList * variable_declaration_list, VariableDeclarationList * prev = NULL);
bool inheritance_loop(SymbolTable * symbol_table, std::string original_class);
bool inheritance_loop(SymbolTable * symbol_table, std::string original_class, std::string current_class);

#endif
