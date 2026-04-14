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
%type <node> program statement expression function_def condition_block if_stmt while_stmt assignment return_stmt
%type <nodeList> statement_list body_content
%type <strList> param_list

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
    condition_block 
    LBRACE body_content RBRACE 
    condition_block
    {
        FunctionNode* func = new FunctionNode(*$2, *$4);
        
        // Add Pre-conditions (from the first condition_block)
        // Note: For simplicity, we'll assume the first block is 'given' 
        // and the second is 'ensure'.
        if ($6) func->addPre(dynamic_cast<ConditionNode*>($6));
        
        // Add Body Statements
        for (auto stmt : *$8) {
            func->addStmt(stmt);
        }
        
        // Add Post-conditions (from the second condition_block)
        if ($10) func->addPost(dynamic_cast<ConditionNode*>($10));
        
        $$ = func;
    }
    ;

param_list:
    /* empty */ { $$ = new std::vector<std::string>(); }
    | IDENTIFIER { $$ = new std::vector<std::string>(); $$->push_back(*$1); }
    | param_list COMMA IDENTIFIER { $1->push_back(*$3); $$ = $1; }
    ;

condition_block:
    /* empty */ { $$ = nullptr; }
    | GIVEN_KWD expression { $$ = new ConditionNode("given", $2); }
    | ENSURE_KWD expression { $$ = new ConditionNode("ensure", $2); }
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
    { 
        // We create an IfNode. For now, we'll pass nullptr for the 'else' branch
        $$ = new IfNode($3, new AssignmentNode("dummy", nullptr), nullptr); 
    }
    ;

while_stmt:
    WHILE_KWD LPAREN expression RPAREN LBRACE statement_list RBRACE
    { 
        // Note: You might need to update your WhileNode constructor in ast.hpp 
        // to take a vector or a single BlockNode.
        $$ = new WhileNode($3, new AssignmentNode("dummy", nullptr)); 
    }
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
    | LPAREN expression RPAREN { $$ = $2; }
    ;

%%

void yyerror(const char *s) {
    std::cerr << "Syntax Error at line " << line_num << ": " << s << std::endl;
}