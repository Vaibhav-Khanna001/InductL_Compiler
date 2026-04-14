%{
#include <iostream>
#include <string>
#include <vector>
#include "ast.hpp"
#include "parser.hpp" 

int line_num = 1;
%}

%option noyywrap

DIGIT       [0-9]
LETTER      [a-zA-Z]
ID          ({LETTER}|_)({LETTER}|{DIGIT}|_)*
NUMBER      {DIGIT}+(\.{DIGIT}+)?
WHITESPACE  [ \t\r]+

%%

\n              { line_num++; }
{WHITESPACE}    { /* Skip */ }
"//".* { /* Skip comments */ }

"func"          { return FUNC_KWD; }
"given"         { return GIVEN_KWD; }
"ensure"        { return ENSURE_KWD; }
"if"            { return IF_KWD; }
"else"          { return ELSE_KWD; }
"while"         { return WHILE_KWD; }
"return"        { return RETURN_KWD; }

{NUMBER}        { yylval.numValue = atof(yytext); return NUMBER_LITERAL; }
{ID}            { yylval.strValue = new std::string(yytext); return IDENTIFIER; }

"=="            { return EQ_OP; }
"!="            { return NE_OP; }
"<="            { return LE_OP; }
">="            { return GE_OP; }
"<"             { return LT_OP; }
">"             { return GT_OP; }
"="             { return ASSIGN; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return STAR; }
"/"             { return SLASH; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
","             { return COMMA; }
";"             { return SEMICOLON; }

.               { std::cerr << "Lexical Error at line " << line_num << ": '" << yytext << "'\n"; }

%%