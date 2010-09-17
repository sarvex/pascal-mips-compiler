%{

#include <stdio.h>
#include "y.tab.hpp"

int line_number = 1;

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

{A}{N}{D}                   return(KW_AND);
{A}{R}{R}{A}{Y}             return(KW_ARRAY);
{B}{E}{G}{I}{N}             return(KW_BEGIN);
{C}{L}{A}{S}{S}             return(KW_CLASS);
{D}{O}                      return(KW_DO);
{E}{L}{S}{E}                return(KW_ELSE);
{E}{N}{D}                   return(KW_END);
{E}{X}{T}{E}{N}{D}{S}       return(KW_EXTENDS);
{F}{U}{N}{C}{T}{I}{O}{N}    return(KW_FUNCTION);
{I}{F}                      return(KW_IF);
{M}{O}{D}                   return(KW_MOD);
{N}{E}{W}                   return(KW_NEW);
{N}{O}{T}                   return(KW_NOT);
{O}{F}                      return(KW_OF);
{O}{R}                      return(KW_OR);
{P}{R}{I}{N}{T}             return(KW_PRINT);
{P}{R}{O}{G}{R}{A}{M}       return(KW_PROGRAM);
{T}{H}{E}{N}                return(KW_THEN);
{V}{A}{R}                   return(KW_VAR);
{W}{H}{I}{L}{E}             return(KW_WHILE);
{T}{H}{I}{S}                return(KW_THIS);
{I}{N}{T}{E}{G}{E}{R}       return(KW_INTEGER);
{B}{O}{O}{L}{E}{A}{N}       return(KW_BOOLEAN);
{R}{E}{A}{L}                return(KW_REAL);
{C}{H}{A}{R}                return(KW_CHAR);
{T}{R}{U}{E}                return(KW_TRUE);
{F}{A}{L}{S}{E}             return(KW_FALSE);

[a-zA-Z][a-zA-Z0-9]*        return(KW_IDENTIFIER);

":="                        return(KW_COLON_EQUAL);
":"                         return(KW_COLON);
","                         return(KW_COMMA);
[0-9]+                      return(KW_DIGIT_SEQUENCE);
"."                         return(KW_DOT);
".."                        return(KW_DOT_DOT);
"="                         return(KW_EQUAL);
">="                        return(KW_GREATER_EQUAL);
">"                         return(KW_GREATER);
"["                         return(KW_LEFT_BRACKET);
"<="                        return(KW_LESS_EQUAL);
"("                         return(KW_LEFT_PARENS);
"<"                         return(KW_LESS);
"-"                         return(KW_MINUS);
"<>"                        return(KW_LESS_GREATER);
"+"                         return(KW_PLUS);
"]"                         return(KW_RIGHT_BRACKET);
")"                         return(KW_RIGHT_PARENS);
";"                         return(KW_SEMICOLON);
"/"                         return(KW_SLASH);
"*"                         return(KW_STAR);

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
            fprintf(stderr, "unexpected EOF inside comment at line %d\n", line_number);
            exit(1);
        }
    }
}

[ \t\f]    ;

\n    line_number++;

.    { fprintf (stderr,
    "'%c' (0%o): illegal character at line %d\n",
     yytext[0], yytext[0], line_number);
    }

%%

int yywrap() {
    return 1;
}

void yyerror(const char *error) {
    printf("%d: %s at '%s'\n", line_number, error, yytext);
    exit(1);
}

