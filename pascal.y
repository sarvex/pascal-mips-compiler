%{
#include "parser.h"

int yylex(void);
void yyerror(const char *error);

extern char *yytext;
extern int line_number;

// main program
Program *main_program;
%}

%expect 1

%token KEYWORD_AND
%token KEYWORD_ARRAY
%token KEYWORD_COLON_EQUAL
%token KEYWORD_CLASS
%token KEYWORD_COLON
%token KEYWORD_COMMA
%token TOKEN_DIGIT_SEQUENCE
%token KEYWORD_DO
%token KEYWORD_DOT
%token KEYWORD_DOT_DOT
%token KEYWORD_ELSE
%token KEYWORD_END
%token KEYWORD_EQUAL
%token KEYWORD_EXTENDS
%token KEYWORD_FUNCTION
%token KEYWORD_GREATER_EQUAL
%token KEYWORD_GREATER
%token KEYWORD_IF
%token KEYWORD_LEFT_BRACKET
%token KEYWORD_LESS_EQUAL
%token KEYWORD_LEFT_PARENS
%token KEYWORD_LESS
%token KEYWORD_MINUS
%token KEYWORD_MOD
%token KEYWORD_NEW
%token KEYWORD_NOT
%token KEYWORD_LESS_GREATER
%token KEYWORD_OF
%token KEYWORD_OR
%token KEYWORD_BEGIN
%token KEYWORD_PLUS
%token KEYWORD_PRINT
%token KEYWORD_PROGRAM
%token KEYWORD_RIGHT_BRACKET
%token KEYWORD_RIGHT_PARENS
%token KEYWORD_SEMICOLON
%token KEYWORD_SLASH
%token KEYWORD_STAR
%token KEYWORD_THEN
%token KEYWORD_THIS
%token KEYWORD_INTEGER
%token KEYWORD_BOOLEAN
%token KEYWORD_REAL
%token KEYWORD_CHAR
%token KEYWORD_TRUE
%token KEYWORD_FALSE
%token KEYWORD_VAR
%token KEYWORD_WHILE

%token <_string> TOKEN_IDENTIFIER

%type <type_denoter> type_denoter
%type <idl> identifier_list
%type <fdes> function_designator
%type <apl> actual_parameter_list
%type <apl> params
%type <ap> actual_parameter
%type <vd> variable_declaration
%type <vdl> variable_declaration_list
%type <fpsl> formal_parameter_section_list
%type <fps> formal_parameter_section
%type <va> variable_access
%type <as> assignment_statement
%type <os> object_instantiation
%type <ps> print_statement
%type <e> expression
%type <s> statement
%type <ss> compound_statement
%type <ss> statement_sequence
%type <is> if_statement
%type <ws> while_statement
%type <iv> indexed_variable
%type <ad> attribute_designator
%type <md> method_designator
%type <el> expression_list
%type <se> simple_expression
%type <t> term
%type <f> factor
%type <_int> sign
%type <p> primary
%type <at> array_type
%type <class_block> class_block
%type <vdl> variable_declaration_part
%type <fdl> func_declaration_list
%type <funcd> function_declaration
%type <fb> function_block
%type <fh> function_heading
%type <fpsl> formal_parameter_list
%type <class_list> class_list
%type <class_declaration> class_declaration
%type <program> program
%type <op> relop
%type <op> addop
%type <op> mulop

%union {
    TypeDenoter *type_denoter;
    char *_string;
    IdentifierList *idl;
    FunctionDesignator *fdes;
    ActualParameterList *apl;
    ActualParameter *ap;
    VariableDeclarationList *vdl;
    VariableDeclaration *vd;
    FormalParameterSectionList *fpsl;
    FormalParameterSection *fps;
    VariableAccess *va;
    AssignmentStatement *as;
    ObjectInstantiation *os;
    PrintStatement *ps;
    Expression *e;
    Statement *s;
    StatementList *ss;
    IfStatement *is;
    WhileStatement *ws;
    IndexedVariable *iv;
    AttributeDesignator *ad;
    MethodDesignator *md;
    ExpressionList *el;
    SimpleExpression *se;
    Term *t;
    Factor *f;
    int * _int;
    Primary *p;
    ArrayType *at;
    ClassBlock * class_block;
    FunctionDeclarationList *fdl;
    FunctionDeclaration *funcd;
    FunctionBlock *fb;
    FunctionHeading *fh;
    ClassList * class_list;
    ClassDeclaration * class_declaration;
    Program * program;
    int op;
}

%%

program : KEYWORD_PROGRAM TOKEN_IDENTIFIER KEYWORD_SEMICOLON class_list KEYWORD_DOT {
    main_program = new Program($2, $4);
};

class_list : class_list class_declaration {
    $$ = new ClassList($2, $1);
} | class_declaration {
    $$ = new ClassList($1, NULL);
};

class_declaration : KEYWORD_CLASS TOKEN_IDENTIFIER KEYWORD_BEGIN class_block KEYWORD_END {
    $$ = new ClassDeclaration($2, NULL, $4);
} | KEYWORD_CLASS TOKEN_IDENTIFIER KEYWORD_EXTENDS TOKEN_IDENTIFIER KEYWORD_BEGIN class_block KEYWORD_END {
    $$ = new ClassDeclaration($2, $4, $6);
};

class_block : variable_declaration_part func_declaration_list {
};

type_denoter : array_type {
} | TOKEN_IDENTIFIER {
} | KEYWORD_INTEGER {
} | KEYWORD_REAL {
} | KEYWORD_CHAR {
} | KEYWORD_BOOLEAN {
};

array_type : KEYWORD_ARRAY KEYWORD_LEFT_BRACKET TOKEN_DIGIT_SEQUENCE KEYWORD_DOT_DOT TOKEN_DIGIT_SEQUENCE KEYWORD_RIGHT_BRACKET KEYWORD_OF type_denoter {
};

variable_declaration_part : KEYWORD_VAR variable_declaration_list KEYWORD_SEMICOLON {
} | {
};

variable_declaration_list : variable_declaration_list KEYWORD_SEMICOLON variable_declaration
	{

	}
 | variable_declaration
	{

	}

 ;

variable_declaration : identifier_list KEYWORD_COLON type_denoter
	{

	}
 ;

func_declaration_list : func_declaration_list KEYWORD_SEMICOLON function_declaration
	{

	}
 | function_declaration
	{

	}
 |
	{

	}
 ;

formal_parameter_list : KEYWORD_LEFT_PARENS formal_parameter_section_list KEYWORD_RIGHT_PARENS 
	{

	}
;
formal_parameter_section_list : formal_parameter_section_list KEYWORD_SEMICOLON formal_parameter_section
	{

	}
 | formal_parameter_section
	{

	}
 ;

formal_parameter_section : identifier_list KEYWORD_COLON TOKEN_IDENTIFIER {
} | KEYWORD_VAR identifier_list KEYWORD_COLON TOKEN_IDENTIFIER {
};

identifier_list : identifier_list KEYWORD_COMMA TOKEN_IDENTIFIER {
} | TOKEN_IDENTIFIER {
};

function_declaration : KEYWORD_FUNCTION TOKEN_IDENTIFIER KEYWORD_SEMICOLON function_block
	{

	}
 | function_heading KEYWORD_SEMICOLON function_block
	{

	}
 ;

function_heading : KEYWORD_FUNCTION TOKEN_IDENTIFIER KEYWORD_COLON type_denoter
	{

	}
 | KEYWORD_FUNCTION TOKEN_IDENTIFIER formal_parameter_list KEYWORD_COLON type_denoter
	{

	}
 ;

function_block : 
  variable_declaration_part
  compound_statement
	{

	}
;

compound_statement : KEYWORD_BEGIN statement_sequence KEYWORD_END
	{

	}
 ;

statement_sequence : statement
	{

	}
 | statement_sequence KEYWORD_SEMICOLON statement
	{

	}
 ;

statement : assignment_statement
	{

	}
 | compound_statement
	{

	}
 | if_statement
	{

	}
 | while_statement
	{

	}
 | print_statement
        {

        }
 ;

while_statement : KEYWORD_WHILE expression KEYWORD_DO statement
	{

	}
 ;

if_statement : KEYWORD_IF expression KEYWORD_THEN statement KEYWORD_ELSE statement
	{

	}
 | KEYWORD_IF expression KEYWORD_THEN statement 
 {

 }
 ;

assignment_statement : variable_access KEYWORD_COLON_EQUAL expression
	{

	}
 | variable_access KEYWORD_COLON_EQUAL object_instantiation
	{

	}
 ;

object_instantiation: KEYWORD_NEW TOKEN_IDENTIFIER
	{

	}
 | KEYWORD_NEW TOKEN_IDENTIFIER params
	{

	}
;

print_statement : KEYWORD_PRINT variable_access
        {

        }
;

variable_access : TOKEN_IDENTIFIER
	{

	}
 | indexed_variable
	{

	}
 | attribute_designator
	{

	}
 | method_designator
	{

	}
 ;

indexed_variable : variable_access KEYWORD_LEFT_BRACKET expression_list KEYWORD_RIGHT_BRACKET
	{

	}
 ;

expression_list : expression_list KEYWORD_COMMA expression
	{

	}
 | expression
	{

	}
 ;

attribute_designator : variable_access KEYWORD_DOT TOKEN_IDENTIFIER
	{

	}
;

method_designator: variable_access KEYWORD_DOT function_designator
	{

	}
 ;


params : KEYWORD_LEFT_PARENS actual_parameter_list KEYWORD_RIGHT_PARENS 
	{

	}
 ;

actual_parameter_list : actual_parameter_list KEYWORD_COMMA actual_parameter
	{

	}
 | actual_parameter 
	{

	}
 ;

actual_parameter : expression
	{

	}
 | expression KEYWORD_COLON expression
	{

	}
 | expression KEYWORD_COLON expression KEYWORD_COLON expression
	{

	}
 ;

expression : simple_expression
	{

	}
 | simple_expression relop simple_expression
	{

	}
 ;

simple_expression : term
	{

	}
 | simple_expression addop term
	{

	}
 ;

term : factor
	{

	}
 | term mulop factor
	{

	}
 ;

sign : KEYWORD_PLUS
	{

	}
 | KEYWORD_MINUS
	{

	}
 ;

factor : sign factor
	{

	}
 | primary 
	{

	}
 ;

primary : variable_access
	{

	}
 | function_designator
	{

	}
 | KEYWORD_LEFT_PARENS expression KEYWORD_RIGHT_PARENS
	{

	}
 | KEYWORD_NOT primary
	{

	}
 ;

/* functions with no params will be handled by plain identifier */
function_designator : TOKEN_IDENTIFIER params
	{

	}
 ;

addop: KEYWORD_PLUS
	{

	}
 | KEYWORD_MINUS
	{

	}
 | KEYWORD_OR
	{

	}
 ;

mulop : KEYWORD_STAR
	{

	}
 | KEYWORD_SLASH
	{

	}
 | KEYWORD_MOD
	{

	}
 | KEYWORD_AND
	{

	}
 ;

relop : KEYWORD_EQUAL
	{

	}
 | KEYWORD_LESS_GREATER
	{

	}
 | KEYWORD_LESS
	{

	}
 | KEYWORD_GREATER
	{

	}
 | KEYWORD_LESS_EQUAL
	{

	}
 | KEYWORD_GREATER_EQUAL
	{

	}
 ;

%%

Program * parse_input() {
    yyparse();
    return main_program;
}

