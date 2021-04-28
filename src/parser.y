%{
#include <cstdio>
#include "ast.hpp"
#include "lexer.hpp"
#include "sem_lib.hpp"

SymbolTable st;
LoggedInfo loggedinfo;
std::vector<int> rt_stack;

LLVMContext AST::TheContext;
IRBuilder<> AST::Builder(TheContext);
std::unique_ptr<Module> AST::TheModule;
std::unique_ptr<legacy::FunctionPassManager> AST::TheFPM;


Function *AST::TheWriteInteger;
Function *AST::TheWriteBoolean;
Function *AST::TheWriteChar;
Function *AST::TheWriteString;
Function *AST::TheWriteReal;
Function *AST::TheReadInteger;
Function *AST::TheReadBoolean;
Function *AST::TheReadChar;
Function *AST::TheReadString;
Function *AST::TheAbs;
Function *AST::TheFabs;
Function *AST::TheSqrt;
Function *AST::TheSin;
Function *AST::TheCos;
Function *AST::TheTan;
Function *AST::TheArctan;
Function *AST::TheExp;
Function *AST::TheLn;
Function *AST::ThePi;
Function *AST::TheTrunc;
Function *AST::TheRound;
Function *AST::TheOrd;
Function *AST::TheChr;

Type *AST::i1 = IntegerType::get(TheContext, 1);
Type *AST::i8 = IntegerType::get(TheContext, 8);
Type *AST::i32 = IntegerType::get(TheContext, 32);
Type *AST::i64 = IntegerType::get(TheContext, 64);
Type *AST::X86_FP80TyID = IntegerType::getX86_FP80Ty(TheContext);


%}

%define parse.error verbose
%verbose
%define parse.trace

%token T_and		"and"
%token T_array		"array"
%token T_begin		"begin"
%token T_boolean	"boolean"
%token T_char		"char"
%token T_dispose	"dispose"
%token T_div		"div"
%token T_do 		"do"
%token T_else		"else"
%token T_end		"end"
%token T_false		"false"
%token T_forward	"forward"
%token T_function	"function"
%token T_goto		"goto"
%token T_if 		"if"
%token T_integer	"integer"
%token T_label		"label"
%token T_mod		"mod"
%token T_new		"new"
%token T_nil		"nil"
%token T_not		"not"
%token T_of 		"of"
%token T_or 		"or"
%token T_procedure	"procedure"
%token T_program 	"program"
%token T_real		"real"
%token T_result		"result"
%token T_return		"return"
%token T_then		"then"
%token T_true		"true"
%token T_var		"var"
%token T_while		"while"

%token T_assign ":="
%token T_colon ":"
%token T_semicolon ";"
%token T_dot "."
%token T_at "@"
%token T_pointer "^"
%token T_openpar "("
%token T_closepar ")"
%token T_openbr "["
%token T_closebr "]"
%token T_comma ","
%token T_eq "="
%token T_less "<"
%token T_greater ">"
%token T_neq "<>"
%token T_geq ">="
%token T_leq "<="
%token T_addition "+"
%token T_subtraction "-"
%token T_multiplication "*"
%token T_division "/"

%token<num> T_intconst
%token<real> T_realconst
%token<ch> T_constchar
%token<str> T_string
%token<var> T_id

%left<op> "or"
%left<op> "and"
%left<op> "not"
%left<op> "=" "<>" "<=" ">=" "<" ">"
%left<op> "+" "-"
%left<op> "*" "/" "div" "mod"
%left<op> "(" ")" "[" "]"
%right<op> "@" "^"

%expect 1

%union{
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
}

%type<body> body
%type<block> block
%type<local> local
%type<local_list> localist
%type<id_list> id_list
%type<header> header
%type<formal> formal
%type<formal_list> headerformal
%type<stmt> stmt call
%type<stmtlist> blockstmt
%type<expr> l-value r-value expr call_as_expr
%type<exprlist> callexpr
%type<decl> decl
%type<decl_list> decl_list
%type<type> type
%type<label> local_label

%%

program:
	"program" T_id ";" body "." { 
		st.openScope();
		st.insertMain();
		Library *lib = new Library();
		lib->init();
        //$4->printOn(std::cout);	// this was used only for debugging
		$4->sem();
		$4->llvm_compile_and_dump();
		st.closeScope();
	}
	;

body: localist block { $$ = new Body($1,$2); }
	;

localist:
	/*nothing*/	{ $$ = new Local_list(); }
	| localist local { $1->append($2); $$ = $1;}
	;

local:
	"var" decl_list { $$ = new Local($2); }
	| "label" local_label { $$ = new Local($2); }
	| header ";" body ";"	{ $$ = new Local($1,$3); }
	| "forward" header ";"	{ $$ = new Local($2); }
	;

local_label:
	T_id id_list ";" { $$ = new Label($1, $2);}
	;

id_list:
	/*nothing*/ { $$ = new Id_list(); }
  	| id_list "," T_id { $1->append_char($3); $$ = $1; }
  	;

decl_list:
	decl { $$ = new Decl_list(); $$->append($1); }
	| decl_list decl { $1->append($2); $$ = $1; }
	;

decl:
  	T_id id_list ":" type ";" { $$ = new Decl($1, $2, $4);}
  	;

header:
	"procedure" T_id "(" formal headerformal ")" { $$ = new Procedure($2,$4,$5); }
	| "procedure" T_id "(" ")" { $$ = new Procedure($2); }
	| "function" T_id "(" formal headerformal ")" ":" type { $$ = new Function_not_from_llvm($2, $4, $5, $8); }
	| "function" T_id "(" ")" ":" type { $$ = new Function_not_from_llvm($2,$6); }
	;

headerformal:
	/*nothing*/ { $$ = new Formal_list(); }
	| headerformal ";" formal { $1->append($3); $$ = $1; }
	;

formal:
	"var" T_id id_list ":" type { $$ = new Formal($2, $3, $5, "var"); }
	| T_id id_list ":" type { $$ = new Formal($1,$2, $4); }
	;

type:
	"integer" {$$ = new Int(); }
	| "real" { $$ = new Real(); }
	| "boolean" { $$ = new Bool(); }
	| "char"  { $$ = new Char(); }
	| "array" "[" T_intconst "]" "of" type { $$ = new Array($6->get_type(), $3); }
    | "array" "of" type { $$ = new Array($3->get_type()); }
    | "^" type { $$ = new Pointer($2->get_type()); }
	;

block:
	"begin" stmt blockstmt "end" {$$ = new Block($2, $3); }
	;

blockstmt:
	/*nothing*/ {$$ = new Stmtlist(); }
	| blockstmt ";" stmt { $1->append($3); $$ = $1; }
	;

stmt:
	/*nothing*/ { $$ = nullptr; }
	| l-value ":=" expr {$$ = new Assign($1,$3); }
	| block { $$ = $1; }
	| call { $$ = $1; }
	| "if" expr "then" stmt { $$ = new If($2, $4); }
	| "if" expr "then" stmt "else" stmt { $$ = new If($2, $4, $6); }
	| "while" expr "do" stmt { $$ = new While($2, $4); }
  	| T_id ":" stmt {$$ = new LabelStmt($1, $3); }
	| "goto" T_id { $$ = new Goto($2); }
	| "return" { $$ = new Return(); }
	| "new" "[" expr "]" l-value { $$ = new New($3, $5); }
	| "new" l-value { $$ = new New($2); }
	| "dispose" "[" "]" l-value { $$ = new Dispose($2, $4); }
	| "dispose" l-value { $$ = new Dispose($2); }
	;

expr:
	l-value { $$ = $1; }
	| r-value { $$ = $1; }
	;

l-value:
	T_id { $$ = new Id($1); }
	| "result" { $$ = new Result(); }
	| T_string { $$ = new Stringconst($1); }
	| l-value "[" expr "]" {$$ = new ArElement($1,$3); }
	| "(" l-value ")" { $$ = $2; }
	| expr "^" { $$ = new Dereference($1); }
	;

r-value:
	T_intconst { $$ = new Intconst($1); }
	| "true" { $$ = new Boolconst("true"); }
	| "false" { $$ = new Boolconst("false"); }
	| T_realconst { $$ = new Realconst($1); }
	| T_constchar { $$ = new Charconst($1); }
	| "(" r-value ")" { $$ = $2; }
	| "nil" { $$ = new Nil(); }
	| "@" expr { $$ = new Reference($2); }
	| "not" expr { $$ = new UnOp($1, $2); }
	| "+" expr { $$ = new UnOp($1, $2); }
	| "-" expr {$$ = new UnOp($1, $2); }
	| expr "+" expr { $$ = new BinOp($1, $2, $3); }
	| expr "-" expr { $$ = new BinOp($1, $2, $3); }
	| expr "*" expr { $$ = new BinOp($1, $2, $3); }
	| expr "/" expr { $$ = new BinOp($1, $2, $3); }
	| expr "div" expr { $$ = new BinOp($1, $2, $3); }
	| expr "mod" expr { $$ = new BinOp($1, $2, $3); }
	| expr "or" expr { $$ = new BinOp($1, $2, $3); }
	| expr "and" expr { $$ = new BinOp($1, $2, $3); }
	| expr "=" expr { $$ = new BinOp($1, $2, $3); }
	| expr "<>" expr { $$ = new BinOp($1, $2, $3); }
	| expr "<" expr { $$ = new BinOp($1, $2, $3); }
	| expr "<=" expr { $$ = new BinOp($1, $2, $3); }
	| expr ">" expr { $$ = new BinOp($1, $2, $3); }
	| expr ">=" expr { $$ = new BinOp($1, $2, $3); }
	| call_as_expr { $$ = $1; }
	;

call:
	T_id "(" expr callexpr ")" { $$ = new Call($1, $3, $4); }
	| T_id "(" ")" { $$ = new Call($1); }
	; 

call_as_expr:
	T_id "(" expr callexpr ")" { $$ = new ExprCall($1, $3, $4); }
	| T_id "(" ")" { $$ = new ExprCall($1); }
	;

callexpr:
	/*nothing*/ { $$ = new Exprlist(); }
	| callexpr "," expr { $1->append($3); $$ = $1;}
	;

%%

int main() {
	int result = yyparse();
	return result;
}