%{
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "ast.h"

extern int yylex();
extern int yylineno;
void yyerror(const char *s);

astNode *root;
%}

/* ---------- Semantic values ---------- */
%union {
    int ival;
    char *sname;
    astNode *node;
    std::vector<astNode*> *nlist;
}

/* ---------- Tokens ---------- */
%token <ival> NUM
%token <sname> IDENTIFIER PRINT READ

%token EXTERN INT VOID IF ELSE WHILE RETURN
%token PLUS MINUS TIMES DIV
%token GT LT GE LE EQ NE
%token ASSIGN
%token LPAREN RPAREN LBRACE RBRACE SEMI

/* ---------- Types ---------- */
%type <node> program function block statement function_call
%type <node> declaration assignment return_stmt
%type <node> if_stmt while_stmt
%type <node> expression additive term factor condition
%type <nlist> decl_list stmt_list

/* ---------- Precedence ---------- */
%left PLUS MINUS
%left TIMES DIV
%nonassoc IFX
%nonassoc ELSE
%nonassoc UNARY

%start program

%%

/* ================= PROGRAM ================= */

program
    : EXTERN VOID PRINT LPAREN INT RPAREN SEMI
      EXTERN INT READ LPAREN RPAREN SEMI
      function
        {
            root = createProg(
                createExtern("print"),
                createExtern("read"),
                $14
            );
        }
    | EXTERN INT READ LPAREN RPAREN SEMI
      EXTERN VOID PRINT LPAREN INT RPAREN SEMI
      function
        {
            root = createProg(
                createExtern("print"),
                createExtern("read"),
                $14
            );
        }
    ;

/* ================= FUNCTION ================= */

function
    : INT IDENTIFIER LPAREN RPAREN block
        { $$ = createFunc($2, NULL, $5); free($2); }

    | INT IDENTIFIER LPAREN INT IDENTIFIER RPAREN block
        { $$ = createFunc($2, createVar($5), $7); free($2); free($5); }
    ;

/* ================= BLOCK ================= */

block
    : LBRACE decl_list stmt_list RBRACE
        {
            $2->insert($2->end(), $3->begin(), $3->end());
            $$ = createBlock($2);
            delete $3;
        }
    | LBRACE decl_list RBRACE
        {
            $$ = createBlock($2);
        }
    ;

/* ================= DECLARATIONS ================= */

decl_list
    : decl_list declaration
        { $$ = $1; $$->push_back($2); }
    | /* empty */
        { $$ = new std::vector<astNode*>(); }
    ;

declaration
    : INT IDENTIFIER SEMI
        { $$ = createDecl($2); free($2); }
    ;

/* ================= STATEMENTS ================= */

stmt_list
    : stmt_list statement
        { $$ = $1; $$->push_back($2); }
    | statement
        { $$ = new std::vector<astNode*>(); $$->push_back($1); }
    ;

statement
    : assignment
    | return_stmt
    | if_stmt
    | while_stmt
    | block
    | function_call SEMI
    ;

assignment
    : IDENTIFIER ASSIGN expression SEMI
        { $$ = createAsgn(createVar($1), $3); free($1); }
    ;

return_stmt
    : RETURN expression SEMI
        { $$ = createRet($2); }
    ;

/* ================= CONTROL ================= */

if_stmt
    : IF LPAREN condition RPAREN statement %prec IFX
        { $$ = createIf($3, $5); }
    | IF LPAREN condition RPAREN statement ELSE statement
        { $$ = createIf($3, $5, $7); }
    ;

while_stmt
    : WHILE LPAREN condition RPAREN statement
        { $$ = createWhile($3, $5); }
    ;

/* ================= EXPRESSIONS ================= */

expression
    : additive
    ;

additive
    : additive PLUS term
        { $$ = createBExpr($1, $3, add); }
    | additive MINUS term
        { $$ = createBExpr($1, $3, sub); }
    | term
        { $$ = $1; }
    ;

term
    : term TIMES factor
        { $$ = createBExpr($1, $3, mul); }
    | term DIV factor
        { $$ = createBExpr($1, $3, divide); }
    | factor
        { $$ = $1; }
    ;

factor
    : NUM
        { $$ = createCnst($1); }
    | IDENTIFIER
        { $$ = createVar($1); free($1); }
    | LPAREN expression RPAREN
        { $$ = $2; }
    | MINUS factor %prec UNARY
        { $$ = createUExpr($2, uminus); }
    | function_call
        { $$ = $1; }
    ;

/* ================= CONDITIONS ================= */

condition
    : additive GT additive { $$ = createRExpr($1,$3,gt); }
    | additive LT additive { $$ = createRExpr($1,$3,lt); }
    | additive GE additive { $$ = createRExpr($1,$3,ge); }
    | additive LE additive { $$ = createRExpr($1,$3,le); }
    | additive EQ additive { $$ = createRExpr($1,$3,eq); }
    | additive NE additive { $$ = createRExpr($1,$3,neq); }
    ;

/* ================= FUNCTION CALLS ================= */

function_call
    : PRINT LPAREN expression RPAREN
        { $$ = createCall($1, $3); free($1); }
    | READ LPAREN RPAREN
        { $$ = createCall($1, NULL); free($1); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax error at line %d\n", yylineno);
}
