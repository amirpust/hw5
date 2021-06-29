%{
    #define YYSTYPE BaseObj*
	#include <stdio.h>
	#include <iostream>
    #include "Enums.hpp"
    #include "RelopAUX.hpp"
    #include "Exp_t.hpp"
    #include "Table.hpp"
    #include "BaseObj.hpp"

    #include "hw3_output.hpp"
    #include "parser.tab.hpp"

    void printLexError();
    void output::errorLex(int yylineno);
    using namespace std;
%}

%option nounput
%option yylineno
%option noyywrap

whitespace	                                        ([ \t\r\n])
noZeroDigit	                                        ([1-9])
digit		                                        ([0-9])
%%
void					                            {return VOID;}
int                                                 {return INT;}
byte                                                {return BYTE;}
b                                                   {return B;}
bool                                                {return BOOL;}
and                                                 {return AND;}
or                                                  {return OR;}
not                                                 {return NOT;}
true                                                {return TRUE;}
false                                               {return FALSE;}
return                                              return RETURN;
if                                                  return IF;
else                                                return ELSE;
while                                               return WHILE;
break                                               return BREAK;
continue                                            return CONTINUE;
switch                                              return SWITCH;
case                                                return CASE;
default                                             return DEFAULT;
:                                                   return COLON;
;                                                   return SC;
,                                                   return COMMA;
\(                                                  return LPAREN;
\)                                                  return RPAREN;
\{                                                  return LBRACE;
\}                                                  return RBRACE;
=                                                   return ASSIGN;
"<"|">"|"<="|">="                                   {yylval = new RelopAUX(string(yytext)); return RELOP;}
"=="|"!="                                           {yylval = new RelopAUX(string(yytext)); return EQUALITY;}
"*"                                                 {return BINOP_MUL;}
"/"                                                 {return BINOP_DIV;}
"+"                                                 {return BINOP_PLUS;}
"-"                                                 {return BINOP_MINUS;}
[a-zA-Z][a-zA-Z0-9]*                                {yylval = new IDtype(string(yytext));  return ID;}
{noZeroDigit}{digit}*                               {yylval = new Num(stoi(yytext)); return NUM;}
0                                                   {yylval = new Num(0); return NUM;}
\"([^\n\r\"\\]|\\[rnt"\\])+\"                       {yylval = new String(yytext); return STRING;}
\/\/[^\r\n]*(\r|\n|\r\n)?                           ;
{whitespace}                                        ;
.                                                   printLexError();
%%

int lineno = yylineno;

void printLexError(){
    output::errorLex(yylineno);
    exit(0);
}