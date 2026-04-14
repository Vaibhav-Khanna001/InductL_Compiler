%{
#include <iostream>
#include <vector>
#include <string>
#include "ast.hpp"

// Global pointer to the root of our AST
ASTNode* root;

// Function prototypes to link with Flex
extern int yylex();
extern int line_num;
void yyerror(const char *s);
%}

/* The Union defines the possible types for Yylval */
%union {
    double numValue;
    std::string* strValue;
    ASTNode* node;
    std::vector<ASTNode*>* nodeList;
    std::vector<std::string>* strList;
    std::vector<ConditionNode*>* condList;
}

/* Define Tokens from Lexer */
%token <numValue> NUMBER_LITERAL
%token <strValue> IDENTIFIER
%token FUNC_KWD GIVEN_KWD ENSURE_KWD
%token IF_KWD ELSE_KWD WHILE_KWD RETURN_KWD
%token ASSIGN PLUS MINUS STAR SLASH
%token EQ_OP NE_OP LE_OP GE_OP LT_OP GT_OP
%token LPAREN RPAREN LBRACE RBRACE COMMA SEMICOLON

/* Define Types for Non-terminals (Rules) */
%type <node> program statement expression function_def if_stmt while_stmt assignment return_stmt
%type <nodeList> statement_list body_content
%type <strList> param_list
/* New types for lists of conditions */
%type <condList> preconditions postconditions

/* Operator Precedence (Standard C-style) */
%left EQ_OP NE_OP LT_OP GT_OP LE_OP GE_OP
%left PLUS MINUS
%left STAR SLASH

%%

/* A program is a list of functions */
program:
    function_def { root = $1; }
    ;

function_def:
    FUNC_KWD IDENTIFIER LPAREN param_list RPAREN 
    preconditions 
    LBRACE body_content RBRACE 
    postconditions
    {
        FunctionNode* func = new FunctionNode(*$2, *$4);
        
        // Add all Pre-conditions collected in the list
        if ($6) {
            for (auto pre : *$6) func->addPre(pre);
            delete $6;
        }
        
        // Add Body Statements
        if ($8) {
            for (auto stmt : *$8) func->addStmt(stmt);
            delete $8;
        }
        
        // Add all Post-conditions collected in the list
        if ($10) {
            for (auto post : *$10) func->addPost(post);
            delete $10;
        }
        
        $$ = func;
    }
    ;

/* Recursive list to handle zero, one, or multiple GIVEN statements */
preconditions:
    /* empty */ { $$ = new std::vector<ConditionNode*>(); }
    | preconditions GIVEN_KWD expression 
      { 
        $1->push_back(new ConditionNode("given", $3)); 
        $$ = $1; 
      }
    ;

/* Recursive list to handle zero, one, or multiple ENSURE statements */
postconditions:
    /* empty */ { $$ = new std::vector<ConditionNode*>(); }
    | postconditions ENSURE_KWD expression 
      { 
        $1->push_back(new ConditionNode("ensure", $3)); 
        $$ = $1; 
      }
    ;

param_list:
    /* empty */ { $$ = new std::vector<std::string>(); }
    | IDENTIFIER { $$ = new std::vector<std::string>(); $$->push_back(*$1); }
    | param_list COMMA IDENTIFIER { $1->push_back(*$3); $$ = $1; }
    ;

body_content:
    statement_list { $$ = $1; }
    ;

statement_list:
    statement { $$ = new std::vector<ASTNode*>(); $$->push_back($1); }
    | statement_list statement { $1->push_back($2); $$ = $1; }
    ;

statement:
    assignment SEMICOLON { $$ = $1; }
    | if_stmt { $$ = $1; }
    | while_stmt { $$ = $1; }
    | return_stmt SEMICOLON { $$ = $1; }
    ;

assignment:
    IDENTIFIER ASSIGN expression { $$ = new AssignmentNode(*$1, $3); }
    ;

if_stmt:
    IF_KWD LPAREN expression RPAREN LBRACE statement_list RBRACE
    { $$ = new IfNode($3, $6); }
    ;

while_stmt:
    WHILE_KWD LPAREN expression RPAREN LBRACE statement_list RBRACE
    { $$ = new WhileNode($3, $6); }
    ;

return_stmt:
    RETURN_KWD expression { $$ = new ReturnNode($2); }
    ;

expression:
    NUMBER_LITERAL { $$ = new NumberNode($1); }
    | IDENTIFIER { $$ = new VariableNode(*$1); }
    | expression PLUS expression { $$ = new BinaryOpNode("+", $1, $3); }
    | expression MINUS expression { $$ = new BinaryOpNode("-", $1, $3); }
    | expression STAR expression { $$ = new BinaryOpNode("*", $1, $3); }
    | expression SLASH expression { $$ = new BinaryOpNode("/", $1, $3); }
    | expression GT_OP expression { $$ = new BinaryOpNode(">", $1, $3); }
    | expression LT_OP expression { $$ = new BinaryOpNode("<", $1, $3); }
    | expression GE_OP expression { $$ = new BinaryOpNode(">=", $1, $3); }
    | expression LE_OP expression { $$ = new BinaryOpNode("<=", $1, $3); }
    | expression EQ_OP expression { $$ = new BinaryOpNode("==", $1, $3); }
    | expression NE_OP expression { $$ = new BinaryOpNode("!=", $1, $3); }
    | LPAREN expression RPAREN { $$ = $2; }
    ;

%%

void yyerror(const char *s) {
    extern char* yytext;
    std::cerr << "Syntax Error at line " << line_num << ": " << s << " (unexpected '" << yytext << "')" << std::endl;
}