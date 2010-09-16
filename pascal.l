%{

#include <stdio.h>
#include "bool.h"
#include "y.tab.h"

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

{A}{N}{D}   return(AND);
{A}{R}{R}{A}{Y}   return(ARRAY);
{C}{L}{A}{S}{S}   return(CLASS);
{D}{O}    return(DO);
{E}{L}{S}{E}   return(ELSE);
{E}{N}{D}   return(END);
{E}{X}{T}{E}{N}{D}{S}   return(EXTENDS);
{F}{U}{N}{C}{T}{I}{O}{N} return(FUNCTION);
{I}{F}    return(IF);
{M}{O}{D}   return(MOD);
{N}{E}{W}    return(NEW);
{N}{O}{T}   return(NOT);
{O}{F}    return(OF);
{O}{R}    return(OR);
{P}{R}{I}{N}{T}  return(PRINT);
{B}{E}{G}{I}{N}   return(PBEGIN);
{P}{R}{O}{G}{R}{A}{M}  return(PROGRAM);
{T}{H}{E}{N}   return(THEN);
{V}{A}{R}   return(VAR);
{W}{H}{I}{L}{E}   return(WHILE);
[a-zA-Z]([a-zA-Z0-9])*  return(IDENTIFIER);

":="    return(COLON_EQUAL);
":"     return(COLON);
","     return(COMMA);
[0-9]+  return(DIGIT_SEQUENCE);
"."     return(DOT);
".."    return(DOT_DOT);
"="     return(EQUAL);
">="    return(GREATER_EQUAL);
">"     return(GREATER);
"["     return(LEFT_BRACE);
"<="    return(LESS_EQUAL);
"("     return(LEFT_PARENS);
"<"     return(LESS);
"-"     return(MINUS);
"<>"    return(LESS_GREATER);
"+"     return(PLUS);
"]"     return(RIGHT_BRACE);
")"     return(RIGHT_PARENS);
";"     return(SEMICOLON);
"/"     return(SLASH);
"*"     return(STAR);
"(*" |
"{"  {
    while (true) {
        int c = input();
        if (c == '}' && yytext[0] == '{') {
            break;
        } else if (c == '*' && yytext[0] == '(') {
            c = input();
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

