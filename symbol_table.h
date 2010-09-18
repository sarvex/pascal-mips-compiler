#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "parser.h"

#include <map>
#include <string>

struct FunctionSymbolTable {
    FunctionDeclaration * function_declaration;
    // function variables, maps variable name to variable declaration
    std::map<std::string, VariableDeclaration *> * variables;

    FunctionSymbolTable(FunctionDeclaration * function_declaration) :
        function_declaration(function_declaration),
        variables(new std::map<std::string, VariableDeclaration *>) {}
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

#endif
