/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NUM = 258,
     IDENTIFIER = 259,
     PRINT = 260,
     READ = 261,
     EXTERN = 262,
     INT = 263,
     VOID = 264,
     IF = 265,
     ELSE = 266,
     WHILE = 267,
     RETURN = 268,
     PLUS = 269,
     MINUS = 270,
     TIMES = 271,
     DIV = 272,
     GT = 273,
     LT = 274,
     GE = 275,
     LE = 276,
     EQ = 277,
     NE = 278,
     ASSIGN = 279,
     LPAREN = 280,
     RPAREN = 281,
     LBRACE = 282,
     RBRACE = 283,
     SEMI = 284,
     IFX = 285,
     UNARY = 286
   };
#endif
/* Tokens.  */
#define NUM 258
#define IDENTIFIER 259
#define PRINT 260
#define READ 261
#define EXTERN 262
#define INT 263
#define VOID 264
#define IF 265
#define ELSE 266
#define WHILE 267
#define RETURN 268
#define PLUS 269
#define MINUS 270
#define TIMES 271
#define DIV 272
#define GT 273
#define LT 274
#define GE 275
#define LE 276
#define EQ 277
#define NE 278
#define ASSIGN 279
#define LPAREN 280
#define RPAREN 281
#define LBRACE 282
#define RBRACE 283
#define SEMI 284
#define IFX 285
#define UNARY 286




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 15 "../frontend/yac.y"
{
    int ival;
    char *sname;
    astNode *node;
    std::vector<astNode*> *nlist;
}
/* Line 1529 of yacc.c.  */
#line 118 "yac.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

