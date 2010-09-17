#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Program;
struct ClassList;
struct ClassIdentification;
struct ClassBlock;
struct TypeDenoter;
struct ArrayType;
struct Range;
struct VariableDeclarationList;
struct VariableDeclaration;
struct FunctionDeclarationList;
struct FunctionDeclaration;
struct FormalParameterSectionList;
struct FormalParameterSection;
struct IdentifierList;
struct FunctionHeading;
struct FunctionBlock;
struct StatementSequence;
struct Statement;
struct WhileStatement;
struct IfStatement;
struct AssignmentStatement;
struct PrintStatement;
struct Expression;
struct ExpressionData;
struct ObjectInstantiation;
struct VariableAccess;
struct IndexedVariable;
struct IndexExpressionList;
struct AttributeDesignator;
struct FunctionDesignator;
struct MethodDesignator;
struct ActualParameterList;
struct ActualParameter;
struct Term;
struct Factor;
struct Primary;
struct UnsignedNumber;



struct Program {
    char *id;
    ClassList * class_list;
    Program(char* id, ClassList * class_list) : id(id), class_list(class_list) {}
};


struct IdentifierList {
  char *id; 
  IdentifierList *next;
};

struct Range{
  UnsignedNumber *min;
  UnsignedNumber *max;
};

struct ArrayType{
  Range *r;
  TypeDenoter *td;
};

#define TYPE_DENOTER_T_ARRAY_TYPE 1
#define TYPE_DENOTER_T_CLASS_TYPE 2
#define TYPE_DENOTER_T_IDENTIFIER 3
struct TypeDenoter{
  int type; /* 1 - array_type
	     * 2 - class_type
	     * 3 - base_type
	     */
  char *name;
  union {
    ArrayType *at;
    ClassList *cl;
    char *id;
  }data;
};

/* ---------------------------------------------------------------- */



/* ----------------------------------------------------------------
 * Everything required for the variable_declaration_list
 * ----------------------------------------------------------------
 */
struct VariableDeclaration {
  IdentifierList *il;
  TypeDenoter *tden;
  int line_number;
};

struct VariableDeclarationList {
  VariableDeclaration *vd;
  VariableDeclarationList *next;  
};
/* ---------------------------------------------------------------- */


/* ----------------------------------------------------------------
 * Everything required for the func_declaration_list
 * ----------------------------------------------------------------
 */
struct FormalParameterSection{
  IdentifierList *il;
  char *id;
  int is_var;
};

struct FormalParameterSectionList{
  FormalParameterSection *fps;
  FormalParameterSectionList *next;
};

struct FunctionHeading{
  char *id;
  char *res; 
  FormalParameterSectionList *fpsl; /* == formal_parameter_list */
};

struct FunctionDeclaration {
  FunctionHeading *fh;
  FunctionBlock *fb;
  int line_number;
};

struct FunctionDeclarationList{
  FunctionDeclaration *fd;
  FunctionDeclarationList *next;
};
/* ---------------------------------------------------------------- */



/* ----------------------------------------------------------------
 * Everything required for the statement_sequence
 * ----------------------------------------------------------------
 */
struct ActualParameter{
  Expression *e1;
  Expression *e2;
  Expression *e3;
};

struct ActualParameterList{
  ActualParameter *ap;
  ActualParameterList *next;
};

struct FunctionDesignator{
  char *id;
  ActualParameterList *apl;
};

/* This is a temporary data structure used to hold the value and type of
   an expression. It is included (inherited) by a bunch of other data
   structures */
struct ExpressionData{
  float val;
  char *type;
};

struct UnsignedNumber{
  int ui;
  ExpressionData *expr;
};

#define PRIMARY_T_VARIABLE_ACCESS 1
#define PRIMARY_T_UNSIGNED_CONSTANT 2
#define PRIMARY_T_FUNCTION_DESIGNATOR 3
#define PRIMARY_T_EXPRESSION 4
#define PRIMARY_T_PRIMARY 5
struct Primary{
  int type; /* 1 - variable_access
	     * 2 - unsigned_constant
	     * 3 - function_designator
	     * 4 - expression
	     * 5 - primary
	     */
  union{
    VariableAccess *va; 
    UnsignedNumber *un; /* == unsigned_constant */
    FunctionDesignator *fd;
    Expression *e;
    struct {
      int _not;
      Primary *next;
    }p;
  }data;
  ExpressionData *expr;
};

#define FACTOR_T_SIGNFACTOR 1
#define FACTOR_T_PRIMARY 2
struct Factor{
  int type; /* 1 - sign/factor
	     * 2 - primary
	     */
  union {
    struct {
      int *sign;
      Factor *next;
    }f;
    Primary *p;
  }data;
  ExpressionData *expr;
};

struct Term{
  Factor *f;
  int mulop;
  ExpressionData *expr;
  Term *next;
};

struct SimpleExpression{
  Term *t;
  int addop;
  ExpressionData *expr;
  SimpleExpression *next;
};

struct Expression{
  SimpleExpression *se1;
  int relop;
  SimpleExpression *se2;
  ExpressionData *expr;
};

struct IndexExpressionList{
  Expression *e;
  IndexExpressionList *next;
  ExpressionData *expr;
};

struct IndexedVariable{
  VariableAccess *va;
  IndexExpressionList *iel;
  ExpressionData *expr;
};

struct AttributeDesignator{
  VariableAccess *va;
  char *id;
};

struct MethodDesignator{
  VariableAccess *va;
  FunctionDesignator *fd;
};

#define VARIABLE_ACCESS_T_IDENTIFIER 1
#define VARIABLE_ACCESS_T_INDEXED_VARIABLE 2
#define VARIABLE_ACCESS_T_ATTRIBUTE_DESIGNATOR 3
#define VARIABLE_ACCESS_T_METHOD_DESIGNATOR 4
struct VariableAccess{
  int type; /* 1 - identifier
	     * 2 - indexed_variable
	     * 3 - attribute_designator
	     * 4 - method_designator
	     */
  union{
    char *id;
    IndexedVariable *iv;
    AttributeDesignator *ad;
    MethodDesignator *md;
  }data;
  char *recordname;          /* This is a temporary field used to collect
				a verbose description of the data type
				that is validated */
  ExpressionData *expr;
};

struct ObjectInstantiation{
  char *id;
  ActualParameterList *apl;
};

struct AssignmentStatement{
  VariableAccess *va;
  Expression *e;  
  ObjectInstantiation *oe;
};

struct IfStatement{
  Expression *e;
  Statement *s1;
  Statement *s2;
};

struct WhileStatement{
  Expression *e;
  Statement *s;
};

struct PrintStatement{
  VariableAccess *va;
};

struct FunctionBlock{
  VariableDeclarationList *vdl;
  StatementSequence *ss;
};

#define STATEMENT_T_ASSIGNMENT 1
#define STATEMENT_T_SEQUENCE 2
#define STATEMENT_T_IF 3
#define STATEMENT_T_WHILE 4
#define STATEMENT_T_PRINT 5
struct Statement {
  int type; /* 1 - assignment_statement
	     * 2 - statement_sequence
	     * 3 - if_statement
	     * 4 - while_statement
	     * 5 - print_statement
	     */

  union{
    AssignmentStatement *as;
    StatementSequence *ss;
    IfStatement *is;
    WhileStatement *ws;
    PrintStatement *ps;
  }data;
  int line_number;
};

struct StatementSequence{
  Statement *s;
  StatementSequence *next;
};

/* ---------------------------------------------------------------- */



/* ----------------------------------------------------------------
 * Everything required for the program
 * ----------------------------------------------------------------
 */
struct ClassIdentification {
  char *id;
  char *extend;
  int line_number;
};

struct ClassBlock{
  VariableDeclarationList *vdl;
  FunctionDeclarationList *fdl;
};

struct ClassList {
  ClassIdentification *ci;
  ClassBlock *cb;
  ClassList *next;
};


Program * parse_input();

#endif

