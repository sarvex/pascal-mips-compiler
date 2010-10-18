#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>


// data structure for Abstract Syntax Tree

struct Program;
struct Identifier;
struct ClassList;
struct ClassDeclaration;
struct ClassBlock;
struct TypeDenoter;
struct ArrayType;
struct VariableDeclarationList;
struct VariableDeclaration;
struct FunctionDeclarationList;
struct FunctionDeclaration;
struct VariableDeclarationList;
struct VariableDeclaration;
struct IdentifierList;
struct FunctionBlock;
struct StatementList;
struct Statement;
struct WhileStatement;
struct IfStatement;
struct AssignmentStatement;
struct PrintStatement;
struct Expression;
struct ComparisonOperator;
struct AdditiveExpression;
struct AdditiveOperator;
struct ObjectInstantiation;
struct VariableAccess;
struct IndexedVariable;
struct ExpressionList;
struct AttributeDesignator;
struct FunctionDesignator;
struct MethodDesignator;
struct MultiplicativeExpression;
struct MultiplicativeOperator;
struct NegatableExpression;
struct PrimaryExpression;
struct LiteralInteger;
struct LiteralReal;
struct LiteralString;
struct LiteralBoolean;



struct Program {
    Identifier * identifier;
    ClassList * class_list;
    Program(Identifier * identifier, ClassList * class_list)
        : identifier(identifier), class_list(class_list) {}
};

struct Identifier {
    std::string text;
    int line_number;
    Identifier(std::string text, int line_number) : text(text), line_number(line_number) {}
};

struct ClassList {
    ClassDeclaration * item;
    ClassList * next;
    ClassList(ClassDeclaration * item, ClassList * next) : item(item), next(next) {}
};

struct ClassDeclaration {
    Identifier * identifier;
    Identifier * parent_identifier;
    ClassBlock * class_block;
    ClassDeclaration(Identifier * identifier, Identifier * parent_identifier, ClassBlock * class_block)
        : identifier(identifier), parent_identifier(parent_identifier), class_block(class_block) {}
};

struct ClassBlock {
    VariableDeclarationList * variable_list;
    FunctionDeclarationList * function_list;
    ClassBlock(VariableDeclarationList * variable_list, FunctionDeclarationList * function_list)
        : variable_list(variable_list), function_list(function_list) {}
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
    Identifier * item;
    IdentifierList * next;
    IdentifierList(Identifier * item, IdentifierList * next)
        : item(item), next(next) {}
};

struct TypeDenoter {
    // don't change this enum without changing the corresponding one in CodeGenerator
    enum Type {INTEGER, REAL, CHAR, BOOLEAN, CLASS, ARRAY};
    Type type;
    union {
        Identifier * class_identifier;
        ArrayType * array_type;
    };
    TypeDenoter(Type type) : type(type) {}
    TypeDenoter(Identifier * class_identifier)
        : type(CLASS), class_identifier(class_identifier) {}
    TypeDenoter(ArrayType * array_type) : type(ARRAY), array_type(array_type) {}
};

struct ArrayType {
    LiteralInteger * min;
    LiteralInteger * max;
    TypeDenoter * type;
    ArrayType(LiteralInteger * min, LiteralInteger * max, TypeDenoter * type)
        : min(min), max(max), type(type) {}
};

struct FunctionDeclarationList {
    FunctionDeclaration * item;
    FunctionDeclarationList * next;
    FunctionDeclarationList(FunctionDeclaration * item, FunctionDeclarationList * next)
        : item(item), next(next) {}
};

struct FunctionDeclaration {
    Identifier * identifier;
    VariableDeclarationList * parameter_list;
    TypeDenoter * type;
    FunctionBlock * block;
    FunctionDeclaration(Identifier * identifier, VariableDeclarationList * parameter_list, TypeDenoter * type, FunctionBlock * block)
        : identifier(identifier), parameter_list(parameter_list), type(type), block(block) {}
};

struct FunctionBlock {
    VariableDeclarationList * variable_list;
    StatementList * statement_list;
    FunctionBlock(VariableDeclarationList * variable_list, StatementList * statement_list)
        : variable_list(variable_list), statement_list(statement_list) {}
};

struct StatementList {
    Statement * item;
    StatementList * next;
    StatementList(Statement * item, StatementList * next) : item(item), next(next) {}
};

struct Statement {
    // don't change this enum without changing the corresponding one in CodeGenerator
    enum Type {FUNCTION, METHOD, ASSIGNMENT, IF, PRINT, WHILE, COMPOUND, ATTRIBUTE};
    Type type;
    union {
        FunctionDesignator * function;
        MethodDesignator * method;
        // hack to have methods with no parentheses
        AttributeDesignator * attribute;
        AssignmentStatement * assignment;
        IfStatement * if_statement;
        PrintStatement * print_statement;
        WhileStatement * while_statement;
        StatementList * compound_statement;
    };
    Statement(FunctionDesignator * function) : type(FUNCTION), function(function) {}
    Statement(MethodDesignator * method) : type(METHOD), method(method) {}
    Statement(AttributeDesignator * attribute) : type(ATTRIBUTE), attribute(attribute) {}
    Statement(AssignmentStatement * assignment) : type(ASSIGNMENT), assignment(assignment) {}
    Statement(IfStatement * if_statement) : type(IF), if_statement(if_statement) {}
    Statement(PrintStatement * print_statement) : type(PRINT), print_statement(print_statement) {}
    Statement(WhileStatement * while_statement) : type(WHILE), while_statement(while_statement) {}
    Statement(StatementList * compound_statement) : type(COMPOUND), compound_statement(compound_statement) {}
};

struct AssignmentStatement {
    VariableAccess * variable;
    Expression * expression;
    AssignmentStatement(VariableAccess * variable, Expression * expression)
        : variable(variable), expression(expression) {}
};

struct IfStatement {
    Expression * expression;
    Statement * then_statement;
    Statement * else_statement;
    IfStatement(Expression * expression, Statement * then_statement, Statement * else_statement)
        : expression(expression), then_statement(then_statement), else_statement(else_statement) {}
};

struct PrintStatement {
    Expression * expression;
    bool trailing_comma;
    PrintStatement(Expression * expression, bool trailing_comma)
        : expression(expression), trailing_comma(trailing_comma) {}
};

struct WhileStatement {
    Expression * expression;
    Statement * statement;
    WhileStatement(Expression * expression, Statement * statement)
        : expression(expression), statement(statement) {}
};

struct VariableAccess {
    // don't change this enum without changing the corresponding one in CodeGenerator
    enum Type {IDENTIFIER, INDEXED_VARIABLE, ATTRIBUTE, THIS};
    Type type;
    union {
        Identifier * identifier;
        IndexedVariable * indexed_variable;
        AttributeDesignator * attribute;
    };
    VariableAccess(Identifier * identifier) : type(IDENTIFIER), identifier(identifier) {}
    VariableAccess(IndexedVariable * indexed_variable)
        : type(INDEXED_VARIABLE), indexed_variable(indexed_variable) {}
    VariableAccess(AttributeDesignator * attribute) : type(ATTRIBUTE), attribute(attribute) {}
    // only valid for THIS
    VariableAccess(Type type) : type(type) {}
};

struct IndexedVariable {
    VariableAccess * variable;
    ExpressionList * expression_list;
    IndexedVariable(VariableAccess * variable, ExpressionList * expression_list)
        : variable(variable), expression_list(expression_list) {}
};

struct ExpressionList {
    Expression * item;
    ExpressionList * next;
    ExpressionList(Expression * item, ExpressionList * next) : item(item), next(next) {}
};

struct Expression {
    AdditiveExpression * left;
    ComparisonOperator * _operator;
    AdditiveExpression * right;
    // annotate this struct with the type when we figure it out
    TypeDenoter * type;
    Expression(AdditiveExpression * only_expression) : left(only_expression), right(NULL) {}
    Expression(AdditiveExpression * left, ComparisonOperator * _operator, AdditiveExpression * right)
        : left(left), _operator(_operator), right(right) {}
};

struct ComparisonOperator {
    // don't change this enum without changing the corresponding one in CodeGenerator
    enum Type {EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL};
    Type type;
    int line_number;
    ComparisonOperator(Type type, int line_number) : type(type), line_number(line_number) {}
};

struct AdditiveExpression {
    AdditiveExpression * left;
    AdditiveOperator * _operator;
    MultiplicativeExpression * right;
    TypeDenoter * type; // cached type
    AdditiveExpression(MultiplicativeExpression * only_expression)
        : left(NULL), right(only_expression) {}
    AdditiveExpression(AdditiveExpression * left, AdditiveOperator * _operator, MultiplicativeExpression * right)
        : left(left), _operator(_operator), right(right) {}
};

struct AdditiveOperator {
    // don't change this enum without changing the corresponding one in CodeGenerator
    enum Type {PLUS, MINUS, OR};
    Type type;
    int line_number;
    AdditiveOperator(Type type, int line_number) : type(type), line_number(line_number) {}
};

struct MultiplicativeExpression {
    MultiplicativeExpression * left;
    MultiplicativeOperator * _operator;
    NegatableExpression * right;

    TypeDenoter * type; // cached type
    MultiplicativeExpression(NegatableExpression * only_expression)
        : left(NULL), right(only_expression) {}
    MultiplicativeExpression(MultiplicativeExpression * left, MultiplicativeOperator * _operator, NegatableExpression * right)
        : left(left), _operator(_operator), right(right) {}
};

struct MultiplicativeOperator {
    // don't change this enum without changing the corresponding one in CodeGenerator
    enum Type {TIMES, DIVIDE, MOD, AND};
    Type type;
    int line_number;
    MultiplicativeOperator(Type type, int line_number) : type(type), line_number(line_number) {}
};

struct NegatableExpression {
    // don't change this enum without changing the corresponding one in CodeGenerator
    enum Type {SIGN, PRIMARY};
    Type type;
    union {
        struct {
            // 1 or -1
            int sign;
            NegatableExpression * next;
        };
        PrimaryExpression * primary_expression;
    };
    TypeDenoter * variable_type;
    NegatableExpression(int sign, NegatableExpression * next) : type(SIGN), sign(sign), next(next) {}
    NegatableExpression(PrimaryExpression * primary_expression)
        : type(PRIMARY), primary_expression(primary_expression) {}
};

struct PrimaryExpression {
    // don't change this enum without changing the corresponding one in CodeGenerator
    enum Type {INTEGER, REAL, STRING, BOOLEAN, VARIABLE, FUNCTION, METHOD, OBJECT_INSTANTIATION, PARENS, NOT};
    int type;
    union {
        LiteralInteger * literal_integer;
        LiteralReal * literal_real;
        LiteralString * literal_string;
        LiteralBoolean * literal_boolean;
        VariableAccess * variable;
        FunctionDesignator * function;
        MethodDesignator * method;
        ObjectInstantiation * object_instantiation;
        Expression * parens_expression;
        PrimaryExpression * not_expression;
    };
    TypeDenoter * variable_type; // cached type
    PrimaryExpression(LiteralInteger * literal_integer) : type(INTEGER), literal_integer(literal_integer) {}
    PrimaryExpression(LiteralReal * literal_real) : type(REAL), literal_real(literal_real) {}
    PrimaryExpression(LiteralString * literal_string) : type(STRING), literal_string(literal_string) {}
    PrimaryExpression(LiteralBoolean * literal_boolean) : type(BOOLEAN), literal_boolean(literal_boolean) {}
    PrimaryExpression(VariableAccess * variable) : type(VARIABLE), variable(variable) {}
    PrimaryExpression(FunctionDesignator * function) : type(FUNCTION), function(function) {}
    PrimaryExpression(MethodDesignator * method) : type(METHOD), method(method) {}
    PrimaryExpression(ObjectInstantiation * object_instantiation)
        : type(OBJECT_INSTANTIATION), object_instantiation(object_instantiation) {}
    PrimaryExpression(Expression * parens_expression)
        : type(PARENS), parens_expression(parens_expression) {}
    PrimaryExpression(PrimaryExpression * not_expression)
        : type(NOT), not_expression(not_expression) {}
};

struct LiteralInteger {
    int value;
    int line_number;
    LiteralInteger(int value, int line_number) : value(value), line_number(line_number) {}
};

struct LiteralReal {
    float value;
    int line_number;
    LiteralReal(float value, int line_number) : value(value), line_number(line_number) {}
};

struct LiteralString {
    std::string value;
    int line_number;
    LiteralString(std::string value, int line_number) : value(value), line_number(line_number) {}
};

struct LiteralBoolean {
    bool value;
    int line_number;
    LiteralBoolean(bool value, int line_number) : value(value), line_number(line_number) {}
};

struct FunctionDesignator {
    Identifier * identifier;
    ExpressionList * parameter_list;
    FunctionDesignator(Identifier * identifier, ExpressionList * parameter_list)
        : identifier(identifier), parameter_list(parameter_list) {}
};


struct AttributeDesignator {
    VariableAccess * owner;
    Identifier * identifier;
    AttributeDesignator(VariableAccess * owner, Identifier * identifier)
        : owner(owner), identifier(identifier) {}
};

struct MethodDesignator {
    VariableAccess * owner;
    FunctionDesignator * function;
    MethodDesignator(VariableAccess * owner, FunctionDesignator * function)
        : owner(owner), function(function) {}
};

struct ObjectInstantiation {
    Identifier * class_identifier;
    ExpressionList * parameter_list;
    ObjectInstantiation(Identifier * class_identifier)
        : class_identifier(class_identifier), parameter_list(NULL) {}
    ObjectInstantiation(Identifier * class_identifier, ExpressionList * parameter_list)
        : class_identifier(class_identifier), parameter_list(parameter_list) {}
};


Program * parse_input(char * filename);

#endif

