#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "parser.h"

#include "insensitive_map.h"
#include <string>

struct FunctionVariable {
    TypeDenoter * type;
    int line_number;

    FunctionVariable(TypeDenoter * type, int line_number) :
        type(type),
        line_number(line_number) {}
};

struct FunctionSymbolTable {
    FunctionDeclaration * function_declaration;
    // function variables, maps variable name to variable declaration
    InsensitiveMap<FunctionVariable *> * variables;

    FunctionSymbolTable(FunctionDeclaration * function_declaration) :
        function_declaration(function_declaration),
        variables(new InsensitiveMap<FunctionVariable *>) {}
};

struct ClassSymbolTable {
    ClassDeclaration * class_declaration;
    // class variables, maps variable name to variable declaration
    InsensitiveMap<VariableDeclaration *> * variables;
    // maps function name to function symbol table
    InsensitiveMap<FunctionSymbolTable *> * function_symbols;

    ClassSymbolTable(ClassDeclaration * class_declaration) :
        class_declaration(class_declaration),
        variables(new InsensitiveMap<VariableDeclaration *>),
        function_symbols(new InsensitiveMap<FunctionSymbolTable *>) {};
};
// maps class name to symbol table
typedef InsensitiveMap<ClassSymbolTable *> SymbolTable;

SymbolTable * build_symbol_table(Program * program);



bool add_variables(InsensitiveMap<FunctionVariable *> * function_variables, VariableDeclaration * variable_declaration);

#endif
