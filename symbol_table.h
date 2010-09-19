#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "parser.h"

#include <map>
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
    std::map<std::string, FunctionVariable *> * variables;

    FunctionSymbolTable(FunctionDeclaration * function_declaration) :
        function_declaration(function_declaration),
        variables(new std::map<std::string, FunctionVariable *>) {}
};

struct ClassSymbolTable {
    ClassDeclaration * class_declaration;
    // class variables, maps variable name to variable declaration
    std::map<std::string, VariableDeclaration *> * variables;
    // maps function name to function symbol table
    std::map<std::string, FunctionSymbolTable *> * function_symbols;

    ClassSymbolTable(ClassDeclaration * class_declaration) :
        class_declaration(class_declaration),
        variables(new std::map<std::string, VariableDeclaration *>),
        function_symbols(new std::map<std::string, FunctionSymbolTable *>) {};
};
// maps class name to symbol table
typedef std::map<std::string, ClassSymbolTable *> SymbolTable;

SymbolTable * build_symbol_table(Program * program);



bool add_variables(std::map<std::string, FunctionVariable *> * function_variables, VariableDeclaration * variable_declaration);

#endif
