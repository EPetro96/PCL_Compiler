/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

#ifndef YY_YY_BUILD_PARSER_HPP_INCLUDED
# define YY_YY_BUILD_PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    T_and = 258,
    T_array = 259,
    T_begin = 260,
    T_boolean = 261,
    T_char = 262,
    T_dispose = 263,
    T_div = 264,
    T_do = 265,
    T_else = 266,
    T_end = 267,
    T_false = 268,
    T_forward = 269,
    T_function = 270,
    T_goto = 271,
    T_if = 272,
    T_integer = 273,
    T_label = 274,
    T_mod = 275,
    T_new = 276,
    T_nil = 277,
    T_not = 278,
    T_of = 279,
    T_or = 280,
    T_procedure = 281,
    T_program = 282,
    T_real = 283,
    T_result = 284,
    T_return = 285,
    T_then = 286,
    T_true = 287,
    T_var = 288,
    T_while = 289,
    T_assign = 290,
    T_colon = 291,
    T_semicolon = 292,
    T_dot = 293,
    T_at = 294,
    T_pointer = 295,
    T_openpar = 296,
    T_closepar = 297,
    T_openbr = 298,
    T_closebr = 299,
    T_comma = 300,
    T_eq = 301,
    T_less = 302,
    T_greater = 303,
    T_neq = 304,
    T_geq = 305,
    T_leq = 306,
    T_addition = 307,
    T_subtraction = 308,
    T_multiplication = 309,
    T_division = 310,
    T_intconst = 311,
    T_realconst = 312,
    T_constchar = 313,
    T_string = 314,
    T_id = 315
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 122 "src/parser.y" /* yacc.c:1909  */

	Body *body;
	Block *block;
	Local *local;
	Local_list *local_list;
	Id_list *id_list;
	Label *label;
	Header *header;
	Formal *formal;
	Formal_list *formal_list;
	Stmt *stmt;
	Stmtlist *stmtlist;
	Expr *expr;
	Exprlist *exprlist;
	Decl *decl;
	Decl_list *decl_list;
	Type_not_from_llvm *type;

	char *var;
	char *ch;
	char *str;
	int num;
	double real;
	char *op;

#line 141 "build/parser.hpp" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_BUILD_PARSER_HPP_INCLUDED  */
