#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>


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
struct AdditiveExpression;
struct ObjectInstantiation;
struct VariableAccess;
struct IndexedVariable;
struct ExpressionList;
struct AttributeDesignator;
struct FunctionDesignator;
struct MethodDesignator;
struct ActualParameterList;
struct ActualParameter;
struct MultiplicativeExpression;
struct NegatableExpression;
struct PrimaryExpression;



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
    enum Type {ASSIGNMENT, IF, PRINT, WHILE, COMPOUND};
    Type type;
    union {
        AssignmentStatement * assignment;
        IfStatement * if_statement;
        PrintStatement * print_statement;
        WhileStatement * while_statement;
        StatementList * compound_statement;
    };
    Statement(AssignmentStatement * assignment) : type(ASSIGNMENT), assignment(assignment) {}
    Statement(IfStatement * if_statement) : type(IF), if_statement(if_statement) {}
    Statement(PrintStatement * print_statement) : type(PRINT), print_statement(print_statement) {}
    Statement(WhileStatement * while_statement) : type(WHILE), while_statement(while_statement) {}
    Statement(StatementList * compound_statement) : type(COMPOUND), compound_statement(compound_statement) {}
};

struct AssignmentStatement {
    VariableAccess * variable;
    Expression * expression;
    AssignmentStatement(VariableAccess * variable, Expression * epxression)
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
    PrintStatement(Expression * expression) : expression(expression) {}
};

struct WhileStatement {
    Expression * expression;
    Statement * statement;
    WhileStatement(Expression * expression, Statement * statement)
        : expression(expression), statement(statement) {}
};

struct VariableAccess {
    enum Type {IDENTIFIER, INDEXED_VARIABLE, ATTRIBUTE};
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
    enum Operator {EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL};
    AdditiveExpression * left;
    Operator _operator;
    AdditiveExpression * right;
    Expression(AdditiveExpression * only_expression) : left(only_expression), right(NULL) {}
    Expression(AdditiveExpression * left, Operator _operator, AdditiveExpression * right)
        : left(left), _operator(_operator), right(right) {}
};

struct AdditiveExpression {
    enum Operator {PLUS, MINUS, OR};
    AdditiveExpression * left;
    Operator _operator;
    MultiplicativeExpression * right;
    AdditiveExpression(MultiplicativeExpression * only_expression)
        : left(NULL), right(only_expression) {}
    AdditiveExpression(AdditiveExpression * left, Operator _operator, MultiplicativeExpression * right)
        : left(left), _operator(_operator), right(right) {}
};

struct MultiplicativeExpression {
    enum Operator {TIMES, DIVIDE, MOD, AND};
    MultiplicativeExpression * left;
    Operator _operator;
    NegatableExpression * right;
    MultiplicativeExpression(NegatableExpression * only_expression)
        : left(NULL), right(only_expression) {}
    MultiplicativeExpression(MultiplicativeExpression * left, Operator _operator, NegatableExpression * right)
        : left(left), _operator(_operator), right(right) {}
};

struct NegatableExpression {
    enum Type {SIGN, PRIMARY};
    Type type;
    union {
        struct {
            int sign;
            NegatableExpression * next;
        };
        PrimaryExpression * primary_expression;
    };
    NegatableExpression(int sign, NegatableExpression * next) : type(SIGN), sign(sign), next(next) {}
    NegatableExpression(PrimaryExpression * primary_expression)
        : type(PRIMARY), primary_expression(primary_expression) {}
};

struct PrimaryExpression {
    enum Type {VARIABLE, FUNCTION, METHOD, OBJECT_INSTANTIATION, PARENS, NOT};
    int type;
    union {
        VariableAccess * variable; 
        FunctionDesignator * function;
        MethodDesignator * method;
        ObjectInstantiation * object_instantiation;
        Expression * parens_expression;
        PrimaryExpression * not_expression;
    };
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

struct FunctionDesignator {
    Identifier * identifier;
    ActualParameterList * parameter_list;
    FunctionDesignator(Identifier * identifier, ActualParameterList * parameter_list)
        : identifier(identifier), parameter_list(parameter_list) {}
};

struct ActualParameterList {
    ActualParameter * item;
    ActualParameterList * next;
    ActualParameterList(ActualParameter * item, ActualParameterList * next) : item(item), next(next) {}
};

struct ActualParameter {
    Expression * expression;
    // only for write and writeln :(
    Expression * special_expression_1;
    Expression * special_expression_2;
    ActualParameter(Expression * expression)
        : expression(expression), special_expression_1(NULL), special_expression_2(NULL) {}
    ActualParameter(Expression * expression, Expression * special_expression_1)
        : expression(expression), special_expression_1(special_expression_1), special_expression_2(NULL) {}
    ActualParameter(Expression * expression, Expression * special_expression_1, Expression * special_expression_2)
        : expression(expression), special_expression_1(special_expression_1), special_expression_2(special_expression_2) {}
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
    ActualParameterList * parameter_list;
    ObjectInstantiation(Identifier * class_identifier)
        : class_identifier(class_identifier), parameter_list(NULL) {}
    ObjectInstantiation(Identifier * class_identifier, ActualParameterList * parameter_list)
        : class_identifier(class_identifier), parameter_list(parameter_list) {}
};


Program * parse_input();

#endif

