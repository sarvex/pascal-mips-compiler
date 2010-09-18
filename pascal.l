%{

#include <cstdlib>

#include "y.tab.hpp"
#include "utils.h"
using Utils::err_header;

int line_number = 1;

void fatal_unexpected(std::string, std::string);

%}

A [aA]
B [bB]
C [cC]
D [dD]
E [eE]
F [fF]
G [gG]
H [hH]
I [iI]
J [jJ]
K [kK]
L [lL]
M [mM]
N [nN]
O [oO]
P [pP]
Q [qQ]
R [rR]
S [sS]
T [tT]
U [uU]
V [vV]
W [wW]
X [xX]
Y [yY]
Z [zZ]

%%

{A}{N}{D}                   return KEYWORD_AND;
{A}{R}{R}{A}{Y}             return KEYWORD_ARRAY;
{B}{E}{G}{I}{N}             return KEYWORD_BEGIN;
{C}{L}{A}{S}{S}             return KEYWORD_CLASS;
{D}{O}                      return KEYWORD_DO;
{E}{L}{S}{E}                return KEYWORD_ELSE;
{E}{N}{D}                   return KEYWORD_END;
{E}{X}{T}{E}{N}{D}{S}       return KEYWORD_EXTENDS;
{F}{U}{N}{C}{T}{I}{O}{N}    return KEYWORD_FUNCTION;
{I}{F}                      return KEYWORD_IF;
{M}{O}{D}                   return KEYWORD_MOD;
{N}{E}{W}                   return KEYWORD_NEW;
{N}{O}{T}                   return KEYWORD_NOT;
{O}{F}                      return KEYWORD_OF;
{O}{R}                      return KEYWORD_OR;
{P}{R}{I}{N}{T}             return KEYWORD_PRINT;
{P}{R}{O}{G}{R}{A}{M}       return KEYWORD_PROGRAM;
{T}{H}{E}{N}                return KEYWORD_THEN;
{V}{A}{R}                   return KEYWORD_VAR;
{W}{H}{I}{L}{E}             return KEYWORD_WHILE;
{T}{H}{I}{S}                return KEYWORD_THIS;
{I}{N}{T}{E}{G}{E}{R}       return KEYWORD_INTEGER;
{B}{O}{O}{L}{E}{A}{N}       return KEYWORD_BOOLEAN;
{R}{E}{A}{L}                return KEYWORD_REAL;
{C}{H}{A}{R}                return KEYWORD_CHAR;

":="                        return KEYWORD_COLON_EQUAL;
":"                         return KEYWORD_COLON;
","                         return KEYWORD_COMMA;
"."                         return KEYWORD_DOT;
".."                        return KEYWORD_DOT_DOT;
"="                         return KEYWORD_EQUAL;
">="                        return KEYWORD_GREATER_EQUAL;
">"                         return KEYWORD_GREATER;
"["                         return KEYWORD_LEFT_BRACKET;
"<="                        return KEYWORD_LESS_EQUAL;
"("                         return KEYWORD_LEFT_PARENS;
"<"                         return KEYWORD_LESS;
"-"                         return KEYWORD_MINUS;
"<>"                        return KEYWORD_LESS_GREATER;
"+"                         return KEYWORD_PLUS;
"]"                         return KEYWORD_RIGHT_BRACKET;
")"                         return KEYWORD_RIGHT_PARENS;
";"                         return KEYWORD_SEMICOLON;
"/"                         return KEYWORD_SLASH;
"*"                         return KEYWORD_STAR;

[0-9]+ {
    yylval.literal_integer = new LiteralInteger(Utils::string_to<int>(yytext), line_number);
    return TOKEN_DIGIT_SEQUENCE;
}
[0-9]+\.[0-9]+ {
    yylval.literal_real = new LiteralReal(Utils::string_to<float>(yytext), line_number);
    return TOKEN_REAL;
}

"'" {
    while (true) {
        int c = yyinput();
        if (c == '\'')
            break;
        else if (c == '\n')
            fatal_unexpected("EOL", "string");
        else if (c == 0) {
            // this is currently impossible because lex hangs
            // if there's no newline at the end of the file
            fatal_unexpected("EOF", "string");
        }
    }
    yylval.literal_string = new LiteralString(yytext, line_number);
    return TOKEN_STRING;
}

{T}{R}{U}{E} {
    yylval.literal_boolean = new LiteralBoolean(true, line_number);
    return TOKEN_LITERAL_BOOLEAN;
}
{F}{A}{L}{S}{E} {
    yylval.literal_boolean = new LiteralBoolean(false, line_number);
    return TOKEN_LITERAL_BOOLEAN;
}

[a-zA-Z][a-zA-Z0-9]* {
    yylval.identifier = new Identifier(yytext, line_number);
    return TOKEN_IDENTIFIER;
}

"(*" |
"{"  {
    while (true) {
        int c = yyinput();
        if (c == '}' && yytext[0] == '{') {
            break;
        } else if (c == '*' && yytext[0] == '(') {
            c = yyinput();
            if (c == ')')
                break;
            else
                unput(c);
        } else if (c == '\n') {
            line_number++;
        } else if (c == 0) {
            fatal_unexpected("EOF", "comment");
        }
    }
}

[ \t\f]    ;

\n    line_number++;

. {
    char c = yytext[0];
    std::cerr << err_header(line_number) << "illegal character '" << c << "' (0x" << std::hex << (int)c << std::dec << ")" << std::endl;
    std::exit(1);
}

%%

int yywrap() {
    return 1;
}

void yyerror(const char *error) {
    std::cerr << err_header(line_number) << error << " at \"" << yytext << "\"" << std::endl;
    std::exit(1);
}

void fatal_unexpected(std::string unexpected_what, std::string inside_what) {
    std::cerr << err_header(line_number) << "unexpected " << unexpected_what << " inside " << inside_what << std::endl;
    std::exit(1);
}

