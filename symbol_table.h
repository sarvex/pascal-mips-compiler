#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "parser.h"

#include "insensitive_map.h"
#include <string>

struct VariableData {
    TypeDenoter * type;
    int line_number;

    VariableData(TypeDenoter * type, int line_number) :
        type(type),
        line_number(line_number) {}
};

struct FunctionSymbolTable {
    FunctionDeclaration * function_declaration;
    // function variables, maps variable name to variable declaration
    InsensitiveMap<VariableData *> * variables;

    FunctionSymbolTable(FunctionDeclaration * function_declaration) :
        function_declaration(function_declaration),
        variables(new InsensitiveMap<VariableData *>) {}
};

struct ClassSymbolTable {
    ClassDeclaration * class_declaration;
    // class variables, maps variable name to variable declaration
    InsensitiveMap<VariableData *> * variables;
    // maps function name to function symbol table
    InsensitiveMap<FunctionSymbolTable *> * function_symbols;

    ClassSymbolTable(ClassDeclaration * class_declaration) :
        class_declaration(class_declaration),
        variables(new InsensitiveMap<VariableData *>),
        function_symbols(new InsensitiveMap<FunctionSymbolTable *>) {};
};
// maps class name to symbol table
typedef InsensitiveMap<ClassSymbolTable *> SymbolTable;

SymbolTable * build_symbol_table(Program * program);



bool add_variables(InsensitiveMap<VariableData *> * function_variables, VariableDeclaration * variable_declaration, std::string function_name);
VariableData * get_field(SymbolTable * symbol_table, std::string class_name, std::string field_name);

#endif
