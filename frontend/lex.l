%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <vector>
    #include "ast.h"
    #include "yac.tab.h"
%}

%%
"int"           { return INT; }
"void"          { return VOID; }
"extern"        { return EXTERN; }
"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"return"        { return RETURN; }

"print"         { yylval.sname = strdup(yytext); return PRINT; }
"read"          { yylval.sname = strdup(yytext); return READ; }

">="            { return GE; }
"<="            { return LE; }
"=="            { return EQ; }
"!="            { return NE; }
">"             { return GT; }
"<"             { return LT; }

[0-9]+          {
                    yylval.ival = atoi(yytext);
                    return NUM;
                }

[a-zA-Z_][a-zA-Z0-9_]* {
                    yylval.sname = strdup(yytext);
                    return IDENTIFIER;
                }

[ \t\n]+        { /* ignore */ }

"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return TIMES; }
"/"             { return DIV; }
"="             { return ASSIGN; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
";"             { return SEMI; }
.               { return yytext[0]; }

%%
int yywrap() {
    return 1;
}
