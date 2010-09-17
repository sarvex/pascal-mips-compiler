%{
#include "not_suck.h"
#include "parser.h"

int yylex(void);
void yyerror(const char *error);

extern char *yytext;
extern int line_number;

// main program
Program *main_program;
%}

%token KW_AND
%token KW_ARRAY
%token KW_COLON_EQUAL
%token KW_CLASS
%token KW_COLON
%token KW_COMMA
%token KW_DIGIT_SEQUENCE
%token KW_DO
%token KW_DOT
%token KW_DOT_DOT
%token KW_ELSE
%token KW_END
%token KW_EQUAL
%token KW_EXTENDS
%token KW_FUNCTION
%token KW_GREATER_EQUAL
%token KW_GREATER
%token KW_IDENTIFIER
%token KW_IF
%token KW_LEFT_BRACE
%token KW_LESS_EQUAL
%token KW_LEFT_PARENS
%token KW_LESS
%token KW_MINUS
%token KW_MOD
%token KW_NEW
%token KW_NOT
%token KW_LESS_GREATER
%token KW_OF
%token KW_OR
%token KW_BEGIN
%token KW_PLUS
%token KW_PRINT
%token KW_PROGRAM
%token KW_RIGHT_BRACE
%token KW_RIGHT_PARENS
%token KW_SEMICOLON
%token KW_SLASH
%token KW_STAR
%token KW_THEN
%token KW_VAR KW_WHILE

%type <_type_denoter> type_denoter
%type <_string> result_type
%type <_string> identifier
%type <idl> identifier_list
%type <fdes> function_designator
%type <apl> actual_parameter_list
%type <apl> params
%type <ap> actual_parameter
%type <vd> variable_declaration
%type <vdl> variable_declaration_list
%type <r> range
%type <un> unsigned_integer
%type <fpsl> formal_parameter_section_list
%type <fps> formal_parameter_section
%type <fps> value_parameter_specification
%type <fps> variable_parameter_specification
%type <va> variable_access
%type <as> assignment_statement
%type <os> object_instantiation
%type <ps> print_statement
%type <e> expression
%type <s> statement
%type <ss> compound_statement
%type <ss> statement_sequence
%type <ss> statement_part
%type <is> if_statement
%type <ws> while_statement
%type <e> boolean_expression
%type <iv> indexed_variable
%type <ad> attribute_designator
%type <md> method_designator
%type <iel> index_expression_list
%type <e> index_expression
%type <se> simple_expression
%type <t> term
%type <f> factor
%type <i> sign
%type <p> primary
%type <un> unsigned_constant
%type <un> unsigned_number
%type <at> array_type
%type <cb> class_block
%type <vdl> variable_declaration_part
%type <fdl> func_declaration_list
%type <funcd> function_declaration
%type <fb> function_block
%type <fh> function_heading
%type <_string> function_identification
%type <fpsl> formal_parameter_list
%type <cl> class_list
%type <_class_identification> class_identification
%type <program> program
%type <op> relop
%type <op> addop
%type <op> mulop

%union {
    struct type_denoter_t *_type_denoter;
    char *_string;
    struct identifier_list_t *idl;
    struct function_designator_t *fdes;
    struct actual_parameter_list_t *apl;
    struct actual_parameter_t *ap;
    struct variable_declaration_list_t *vdl;
    struct variable_declaration_t *vd;
    struct range_t *r;
    struct unsigned_number_t *un;
    struct formal_parameter_section_list_t *fpsl;
    struct formal_parameter_section_t *fps;
    struct variable_access_t *va;
    struct assignment_statement_t *as;
    struct object_instantiation_t *os;
    struct print_statement_t *ps;
    struct expression_t *e;
    struct statement_t *s;
    struct statement_sequence_t *ss;
    struct if_statement_t *is;
    struct while_statement_t *ws;
    struct indexed_variable_t *iv;
    struct attribute_designator_t *ad;
    struct method_designator_t *md;
    struct index_expression_list_t *iel;
    struct simple_expression_t *se;
    struct term_t *t;
    struct factor_t *f;
    int *i;
    struct primary_t *p;
    struct array_type_t *at;
    struct class_block_t *cb;
    struct func_declaration_list_t *fdl;
    struct function_declaration_t *funcd;
    struct function_block_t *fb;
    struct function_heading_t *fh;
    struct class_identification_t *_class_identification;
    struct class_list_t *cl;
    Program * program;
    int op;
}

%%

program : KW_PROGRAM identifier semicolon class_list KW_DOT {
    main_program = new_program($2, $4);
};

class_list : class_list class_identification KW_BEGIN class_block KW_END {
} | class_identification KW_BEGIN class_block KW_END {
};

class_identification : KW_CLASS identifier {
} | KW_CLASS identifier KW_EXTENDS identifier {
};

class_block : variable_declaration_part func_declaration_list {
};

type_denoter : array_type {
} | identifier {
};

array_type : KW_ARRAY KW_LEFT_BRACE range KW_RIGHT_BRACE KW_OF type_denoter {
};

range : unsigned_integer KW_DOT_DOT unsigned_integer {
};

variable_declaration_part : KW_VAR variable_declaration_list semicolon {
} | {
};

variable_declaration_list : variable_declaration_list semicolon variable_declaration
	{

	}
 | variable_declaration
	{

	}

 ;

variable_declaration : identifier_list KW_COLON type_denoter
	{

	}
 ;

func_declaration_list : func_declaration_list semicolon function_declaration
	{

	}
 | function_declaration
	{

	}
 |
	{

	}
 ;

formal_parameter_list : KW_LEFT_PARENS formal_parameter_section_list KW_RIGHT_PARENS 
	{

	}
;
formal_parameter_section_list : formal_parameter_section_list semicolon formal_parameter_section
	{

	}
 | formal_parameter_section
	{

	}
 ;

formal_parameter_section : value_parameter_specification
 | variable_parameter_specification
 ;

value_parameter_specification : identifier_list KW_COLON identifier
	{

	}
 ;

identifier_list : identifier_list comma identifier {
} | identifier {
};

variable_parameter_specification : KW_VAR identifier_list KW_COLON identifier
	{

	}
 ;

function_declaration : function_identification semicolon function_block
	{

	}
 | function_heading semicolon function_block
	{

	}
 ;

function_heading : KW_FUNCTION identifier KW_COLON result_type
	{

	}
 | KW_FUNCTION identifier formal_parameter_list KW_COLON result_type
	{

	}
 ;

result_type : identifier ;

function_identification : KW_FUNCTION identifier
	{

	}
;

function_block : 
  variable_declaration_part
  statement_part
	{

	}
;

statement_part : compound_statement
 ;

compound_statement : KW_BEGIN statement_sequence KW_END
	{

	}
 ;

statement_sequence : statement
	{

	}
 | statement_sequence semicolon statement
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

while_statement : KW_WHILE boolean_expression KW_DO statement
	{

	}
 ;

if_statement : KW_IF boolean_expression KW_THEN statement KW_ELSE statement
	{

	}
 ;

assignment_statement : variable_access KW_COLON_EQUAL expression
	{

	}
 | variable_access KW_COLON_EQUAL object_instantiation
	{

	}
 ;

object_instantiation: KW_NEW identifier
	{

	}
 | KW_NEW identifier params
	{

	}
;

print_statement : KW_PRINT variable_access
        {

        }
;

variable_access : identifier
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

indexed_variable : variable_access KW_LEFT_BRACE index_expression_list KW_RIGHT_BRACE
	{

	}
 ;

index_expression_list : index_expression_list comma index_expression
	{

	}
 | index_expression
	{

	}
 ;

index_expression : expression ;

attribute_designator : variable_access KW_DOT identifier
	{

	}
;

method_designator: variable_access KW_DOT function_designator
	{

	}
 ;


params : KW_LEFT_PARENS actual_parameter_list KW_RIGHT_PARENS 
	{

	}
 ;

actual_parameter_list : actual_parameter_list comma actual_parameter
	{

	}
 | actual_parameter 
	{

	}
 ;

actual_parameter : expression
	{

	}
 | expression KW_COLON expression
	{

	}
 | expression KW_COLON expression KW_COLON expression
	{

	}
 ;

boolean_expression : expression ;

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

sign : KW_PLUS
	{

	}
 | KW_MINUS
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
 | unsigned_constant
	{

	}
 | function_designator
	{

	}
 | KW_LEFT_PARENS expression KW_RIGHT_PARENS
	{

	}
 | KW_NOT primary
	{

	}
 ;

unsigned_constant : unsigned_number
 ;

unsigned_number : unsigned_integer ;

unsigned_integer : KW_DIGIT_SEQUENCE
	{

	}
 ;

/* functions with no params will be handled by plain identifier */
function_designator : identifier params
	{

	}
 ;

addop: KW_PLUS
	{

	}
 | KW_MINUS
	{

	}
 | KW_OR
	{

	}
 ;

mulop : KW_STAR
	{

	}
 | KW_SLASH
	{

	}
 | KW_MOD
	{

	}
 | KW_AND
	{

	}
 ;

relop : KW_EQUAL
	{

	}
 | KW_LESS_GREATER
	{

	}
 | KW_LESS
	{

	}
 | KW_GREATER
	{

	}
 | KW_LESS_EQUAL
	{

	}
 | KW_GREATER_EQUAL
	{

	}
 ;

identifier : KW_IDENTIFIER
	{

	}
 ;

semicolon : KW_SEMICOLON
 ;

comma : KW_COMMA
 ;

%%

void parse_input() {
    yyparse();
}

