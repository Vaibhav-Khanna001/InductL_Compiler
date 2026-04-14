/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_SRC_PARSER_HPP_INCLUDED
# define YY_YY_SRC_PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    NUMBER_LITERAL = 258,          /* NUMBER_LITERAL  */
    IDENTIFIER = 259,              /* IDENTIFIER  */
    FUNC_KWD = 260,                /* FUNC_KWD  */
    GIVEN_KWD = 261,               /* GIVEN_KWD  */
    ENSURE_KWD = 262,              /* ENSURE_KWD  */
    IF_KWD = 263,                  /* IF_KWD  */
    ELSE_KWD = 264,                /* ELSE_KWD  */
    WHILE_KWD = 265,               /* WHILE_KWD  */
    RETURN_KWD = 266,              /* RETURN_KWD  */
    ASSIGN = 267,                  /* ASSIGN  */
    PLUS = 268,                    /* PLUS  */
    MINUS = 269,                   /* MINUS  */
    STAR = 270,                    /* STAR  */
    SLASH = 271,                   /* SLASH  */
    EQ_OP = 272,                   /* EQ_OP  */
    NE_OP = 273,                   /* NE_OP  */
    LE_OP = 274,                   /* LE_OP  */
    GE_OP = 275,                   /* GE_OP  */
    LT_OP = 276,                   /* LT_OP  */
    GT_OP = 277,                   /* GT_OP  */
    LPAREN = 278,                  /* LPAREN  */
    RPAREN = 279,                  /* RPAREN  */
    LBRACE = 280,                  /* LBRACE  */
    RBRACE = 281,                  /* RBRACE  */
    COMMA = 282,                   /* COMMA  */
    SEMICOLON = 283                /* SEMICOLON  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 17 "src/parser.y"

    double numValue;
    std::string* strValue;
    ASTNode* node;
    std::vector<ASTNode*>* nodeList;
    std::vector<std::string>* strList;

#line 100 "src/parser.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_SRC_PARSER_HPP_INCLUDED  */
