%option noyywrap
%x COMMENT

%{
#define T_eof 0
%}

%{
#include <cstdio>
#include <cstdlib>
#include "lexer.hpp"
#include "ast.hpp"
#include "parser.hpp"
%}

C [A-Z]
P [a-z]
D [0-9]
W "\\n"|"\\t"|"\\r"|"\\0"|"\\\\"|"\\\'"|"\\\""
PRINTABLE [^["'\]]
S [^" ^' ^\ ^"\n"]

%%

"and"	{ yylval.op = strdup(yytext); return T_and; }
"do"	{ return T_do; }
"if"	{ return T_if; }
"of"	{ return T_of; }
"then"	{ return T_then; }
"array"	{ return T_array; }
"else"	{ return T_else; }
"integer"	{ return T_integer; }
"or"	{ yylval.op = strdup(yytext); return T_or; }
"true"	{ return T_true; }
"begin"	{ return T_begin; }
"end"	{ return T_end; }
"label"	{ return T_label; }
"procedure"	{ return T_procedure; }
"var"	{ return T_var; }
"boolean"	{ return T_boolean; }
"false"	{ return T_false; }
"mod"	{ yylval.op = strdup(yytext); return T_mod; }
"program"	{ return T_program; }
"while"	{ return T_while; }
"char"	{ return T_char; }
"forward"	{ return T_forward; }
"new"	{ return T_new; }
"real"	{ return T_real; }
"dispose"	{ return T_dispose; }
"function"	{ return T_function; }
"nil"	{ return T_nil; }
"result"	{ return T_result; }
"div"	{ yylval.op = strdup(yytext); return T_div; }
"goto"	{ return T_goto; }
"not"	{ yylval.op = strdup(yytext); return T_not; }
"return"	{ return T_return; }

({C}|{P})({C}|{P}|{D}|"_")*	{yylval.var = strdup(yytext); return T_id; }
{D}+	{yylval.num = std::stoi(yytext); return T_intconst; }
({D}+)("."{D}+)([eE]("+"|"-")?{D}+)?	{yylval.real = std::stod(yytext); return T_realconst; }


\'({W}|{PRINTABLE})\'	{yylval.ch = strdup(yytext); return T_constchar; }

\"(\\.|[^\\"^\n])*\"	{ yylval.str = escapeString(yytext); return T_string; }


"="			{ yylval.op = strdup(yytext); return T_eq; }
">"			{ yylval.op = strdup(yytext); return T_less; }
"<"			{ yylval.op = strdup(yytext); return T_greater; }
"<>"		{ yylval.op = strdup(yytext); return T_neq; }
">="		{ yylval.op = strdup(yytext); return T_geq; }
"<="		{ yylval.op = strdup(yytext); return T_leq; }
"+"			{ yylval.op = strdup(yytext); return T_addition; }
"-"			{ yylval.op = strdup(yytext); return T_subtraction; }
"*"			{ yylval.op = strdup(yytext); return T_multiplication; }
"/"			{ yylval.op = strdup(yytext); return T_division; }
"^"			{ return T_pointer; }
"@"			{ return T_at; }
";"			{ return T_semicolon; }
"("			{ return T_openpar; }
")"			{ return T_closepar; }
"."			{ return T_dot; }
","			{ return T_comma; }
":"			{ return T_colon; }
":="		{ return T_assign; }
"["			{ return T_openbr; }
"]"			{ return T_closebr; }


[ \n\t\r]+ { /* nothing */ }

"(*"	{ BEGIN(COMMENT); }
<COMMENT>"*)"	{ BEGIN(INITIAL); }
<COMMENT>.	{ }
<COMMENT>\n	{ }

.	{ yyerror("lexical error");	}

%%
/*
int main () {
  int token;
  do {
    token = yylex();
    printf("token=%d, lexeme=\"%s\"\n", token, yytext);
  } while (token != T_eof);
}
*/

void yyerror(const char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(1);
}