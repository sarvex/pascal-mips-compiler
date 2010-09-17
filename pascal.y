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
%type <identifier_list> identifier_list
%type <function_designator> function_designator
%type <actual_parameter_list> actual_parameter_list
%type <actual_parameter_list> params
%type <actual_parameter> actual_parameter
%type <variable_declaration> variable_declaration
%type <variable_declaration_list> variable_declaration_list
%type <formal_parameter_section_list> formal_parameter_section_list
%type <formal_parameter_section> formal_parameter_section
%type <variable_access> variable_access
%type <assignment_statment> assignment_statement
%type <object_instantiation> object_instantiation
%type <print_statement> print_statement
%type <expressoin> expression
%type <statement> statement
%type <statement_list> compound_statement
%type <statement_list> statement_sequence
%type <if_statement> if_statement
%type <while_statement> while_statement
%type <indexed_variable> indexed_variable
%type <attribute_designator> attribute_designator
%type <method_designator> method_designator
%type <expression_list> expression_list
%type <additive_expression> additive_expression
%type <multiplicative_expression> multiplicative_expression
%type <negatable_expression> negatable_expression
%type <_int> sign
%type <primary_expression> primary_expression
%type <array_type> array_type
%type <class_block> class_block
%type <variable_declaration_list> variable_declaration_part
%type <function_declaration_list> func_declaration_list
%type <function_declaration> function_declaration
%type <function_block> function_block
%type <function_heading> function_heading
%type <formal_parameter_section_list> formal_parameter_list
%type <class_list> class_list
%type <class_declaration> class_declaration
%type <program> program
%type <comparison_operator> comparison_operator
%type <additive_operator> additive_operator
%type <multiplicative_operator> multiplicative_operator

%union {
    TypeDenoter * type_denoter;
    char * _string;
    IdentifierList * identifier_list;
    FunctionDesignator * function_designator;
    ActualParameterList * actual_parameter_list;
    ActualParameter * actual_parameter;
    VariableDeclarationList * variable_declaration_list;
    VariableDeclaration * variable_declaration;
    FormalParameterSectionList * formal_parameter_section_list;
    FormalParameterSection * formal_parameter_section;
    VariableAccess * variable_access;
    AssignmentStatement * assignment_statment;
    ObjectInstantiation * object_instantiation;
    PrintStatement * print_statement;
    Expression * expressoin;
    Statement * statement;
    StatementList * statement_list;
    IfStatement * if_statement;
    WhileStatement * while_statement;
    IndexedVariable * indexed_variable;
    AttributeDesignator * attribute_designator;
    MethodDesignator * method_designator;
    ExpressionList * expression_list;
    AdditiveExpression * additive_expression;
    MultiplicativeExpression * multiplicative_expression;
    NegatableExpression * negatable_expression;
    int * _int;
    PrimaryExpression * primary_expression;
    ArrayType * array_type;
    ClassBlock * class_block;
    FunctionDeclarationList * function_declaration_list;
    FunctionDeclaration * function_declaration;
    FunctionBlock * function_block;
    FunctionHeading * function_heading;
    ClassList * class_list;
    ClassDeclaration * class_declaration;
    Program * program;
    Expression::Operator comparison_operator;
    AdditiveExpression::Operator additive_operator;
    MultiplicativeExpression::Operator multiplicative_operator;
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
    $$ = new ClassBlock($1, $2);
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

statement : assignment_statement {
} | compound_statement {
} | if_statement {
} | while_statement {
} | print_statement {
};

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

assignment_statement : variable_access KEYWORD_COLON_EQUAL expression {
};

object_instantiation: KEYWORD_NEW TOKEN_IDENTIFIER {
} | KEYWORD_NEW TOKEN_IDENTIFIER params {
};

print_statement : KEYWORD_PRINT expression {
};

variable_access : TOKEN_IDENTIFIER {
} | indexed_variable {
} | attribute_designator {
};

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

attribute_designator : variable_access KEYWORD_DOT TOKEN_IDENTIFIER {
};

method_designator: variable_access KEYWORD_DOT function_designator {
};


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

actual_parameter : expression {
} | expression KEYWORD_COLON expression {
} | expression KEYWORD_COLON expression KEYWORD_COLON expression {
};

expression : additive_expression {
} | additive_expression comparison_operator additive_expression {
};

additive_expression : multiplicative_expression {
} | additive_expression additive_operator multiplicative_expression {
};

multiplicative_expression : negatable_expression {
} | multiplicative_expression multiplicative_operator negatable_expression {
};

sign : KEYWORD_PLUS {
} | KEYWORD_MINUS {
};

negatable_expression : sign negatable_expression {
} | primary_expression {
};

primary_expression : variable_access {
} | function_designator {
} | method_designator {
} | object_instantiation {
} | KEYWORD_LEFT_PARENS expression KEYWORD_RIGHT_PARENS {
} | KEYWORD_NOT primary_expression {
};

/* functions with no params will be handled by plain identifier */
function_designator : TOKEN_IDENTIFIER params {
};

additive_operator : KEYWORD_PLUS {
} | KEYWORD_MINUS {
} | KEYWORD_OR {
};

multiplicative_operator : KEYWORD_STAR {
} | KEYWORD_SLASH {
} | KEYWORD_MOD {
} | KEYWORD_AND {
};

comparison_operator : KEYWORD_EQUAL {
} | KEYWORD_LESS_GREATER {
} | KEYWORD_LESS {
} | KEYWORD_GREATER {
} | KEYWORD_LESS_EQUAL {
} | KEYWORD_GREATER_EQUAL {
};

%%

Program * parse_input() {
    yyparse();
    return main_program;
}

