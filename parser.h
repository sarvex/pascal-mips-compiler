#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Program;
struct ClassList;
struct ClassDeclaration;
struct ClassBlock;
struct TypeDenoter;
struct ArrayType;
struct VariableDeclarationList;
struct VariableDeclaration;
struct FunctionDeclarationList;
struct FunctionDeclaration;
struct FormalParameterSectionList;
struct FormalParameterSection;
struct IdentifierList;
struct FunctionHeading;
struct FunctionBlock;
struct StatementList;
struct Statement;
struct WhileStatement;
struct IfStatement;
struct AssignmentStatement;
struct PrintStatement;
struct Expression;
struct SimpleExpression;
struct ObjectInstantiation;
struct VariableAccess;
struct IndexedVariable;
struct ExpressionList;
struct AttributeDesignator;
struct FunctionDesignator;
struct MethodDesignator;
struct ActualParameterList;
struct ActualParameter;
struct Term;
struct Factor;
struct Primary;



struct Program {
    char * id;
    ClassList * class_list;
    Program(char* id, ClassList * class_list) : id(id), class_list(class_list) {}
};

struct ClassList {
    ClassDeclaration * item;
    ClassList * next;
    ClassList(ClassDeclaration * item, ClassList * next) : item(item), next(next) {}
};

struct ClassDeclaration {
    char * id;
    char * parent_id;
    ClassBlock * class_block;
    ClassDeclaration(char * id, char * parent_id, ClassBlock * class_block)
        : id(id), parent_id(parent_id), class_block(class_block) {}
};

struct ClassBlock {
    VariableDeclarationList * variables;
    FunctionDeclarationList * functions;
    ClassBlock(VariableDeclarationList * variables, FunctionDeclarationList * functions)
        : variables(variables), functions(functions) {}
};

struct VariableDeclarationList {
    VariableDeclaration * item;
    VariableDeclarationList * next;  
    VariableDeclarationList(VariableDeclaration * item, VariableDeclarationList * next)
        : item(item), next(next) {}
};

struct VariableDeclaration {
    IdentifierList * id_list;
    TypeDenoter * type;
    VariableDeclaration(IdentifierList * id_list, TypeDenoter * type)
        : id_list(id_list), type(type) {}
};

struct IdentifierList {
    char * item; 
    IdentifierList * next;
    IdentifierList(char * item, IdentifierList * next)
        : item(item), next(next) {}
};

struct TypeDenoter {
    enum Type {INTEGER, REAL, CHAR, BOOLEAN, CLASS, ARRAY};
    Type type;
    union {
        char * class_id;
        ArrayType * array_type;
    };
    TypeDenoter(Type type) : type(type) {}
    TypeDenoter(char * class_id) : type(CLASS), class_id(class_id) {}
    TypeDenoter(ArrayType * array_type) : type(ARRAY), array_type(array_type) {}
};

struct ArrayType {
    int min;
    int max;
    TypeDenoter * type;
    ArrayType(int min, int max, TypeDenoter * type) : min(min), max(max), type(type) {}
};

struct FunctionDeclarationList {
    FunctionDeclaration * item;
    FunctionDeclarationList * next;
    FunctionDeclarationList(FunctionDeclaration * item, FunctionDeclarationList * next)
        : item(item), next(next) {}
};

struct FunctionDeclaration {
    FunctionHeading * heading;
    FunctionBlock * block;
    FunctionDeclaration(FunctionHeading * heading, FunctionBlock * block)
        : heading(heading), block(block) {}
};

struct FunctionHeading {
    char * id;
    FormalParameterSectionList * parameter_list;
    FunctionHeading(char * id, FormalParameterSectionList * parameter_list)
        : id(id), parameter_list(parameter_list) {}
};

struct FormalParameterSectionList {
    FormalParameterSection * item;
    FormalParameterSectionList * next;
    FormalParameterSectionList(FormalParameterSection * item, FormalParameterSectionList * next)
        : item(item), next(next) {}
};

struct FormalParameterSection {
    IdentifierList * id_list;
    TypeDenoter * type;
    FormalParameterSection(IdentifierList * id_list, TypeDenoter * type)
        : id_list(id_list), type(type) {}
};

struct FunctionBlock {
    VariableDeclarationList * variables;
    StatementList * statements;
    FunctionBlock(VariableDeclarationList * variables, StatementList * statements)
        : variables(variables), statements(statements) {}
};

struct StatementList {
    Statement * item;
    StatementList * next;
    StatementList(Statement * item, StatementList * next) : item(item), next(next) {}
};

struct Statement {
    enum Type {ASSIGNMENT, IF, PRINT, WHILE, COMPOUND};
    Type type;
    union {
        AssignmentStatement * assignment;
        IfStatement * if_statement;
        PrintStatement * print_statement;
        WhileStatement * while_statement;
        StatementList * compound_statement;
    };
};

struct AssignmentStatement {
    VariableAccess *va;
    Expression *e;  
    ObjectInstantiation *oe;
};

struct ObjectInstantiation {
    char *id;
    ActualParameterList *apl;
};

struct IfStatement {
    Expression *e;
    Statement *s1;
    Statement *s2;
};

struct PrintStatement {
    VariableAccess *va;
};

struct WhileStatement {
    Expression *e;
    Statement *s;
};

#define VARIABLE_ACCESS_T_IDENTIFIER 1
#define VARIABLE_ACCESS_T_INDEXED_VARIABLE 2
#define VARIABLE_ACCESS_T_ATTRIBUTE_DESIGNATOR 3
#define VARIABLE_ACCESS_T_METHOD_DESIGNATOR 4
struct VariableAccess {
    int type;
    union {
        char *id;
        IndexedVariable *iv;
        AttributeDesignator *ad;
        MethodDesignator *md;
    } data;
    char *recordname;
};

struct IndexedVariable {
    VariableAccess *va;
    ExpressionList *expression_list;
};

struct ExpressionList {
    Expression *e;
    ExpressionList *next;
};

struct Expression {
    SimpleExpression *se1;
    int relop;
    SimpleExpression *se2;
};

struct SimpleExpression {
    Term *t;
    int addop;
    SimpleExpression *next;
};

struct Term {
    Factor *f;
    int mulop;
    Term *next;
};

#define FACTOR_T_SIGNFACTOR 1
#define FACTOR_T_PRIMARY 2
struct Factor {
    int type;
    union {
        struct {
            int *sign;
            Factor *next;
        } f;
        Primary *p;
    } data;
};

#define PRIMARY_T_VARIABLE_ACCESS 1
#define PRIMARY_T_UNSIGNED_CONSTANT 2
#define PRIMARY_T_FUNCTION_DESIGNATOR 3
#define PRIMARY_T_EXPRESSION 4
#define PRIMARY_T_PRIMARY 5
struct Primary {
    int type;
    union {
        VariableAccess *va; 
        int un;
        FunctionDesignator *fd;
        Expression *e;
        struct {
            int _not;
            Primary *next;
        } p;
    } data;
};

struct FunctionDesignator {
    char *id;
    ActualParameterList *apl;
};

struct ActualParameterList {
    ActualParameter *ap;
    ActualParameterList *next;
};

struct ActualParameter {
    Expression *e1;
    Expression *e2;
    Expression *e3;
};


struct AttributeDesignator {
    VariableAccess *va;
    char *id;
};

struct MethodDesignator {
    VariableAccess *va;
    FunctionDesignator *fd;
};






Program * parse_input();

#endif

