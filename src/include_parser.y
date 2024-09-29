%{
#include "seagoo.h"

void yyerror(const char *s);
int yylex(void);

%}

%union {
    char *str;
}

%token <str> INCLUDE
%token <str> STRING_LITERAL
%type <str> string

%%
input:
    | input include_stmt
    ;

include_stmt:
    INCLUDE string {
        printf("Found include: %s\n", $2);
        free($2);
    }
    | INCLUDE error {
        yyerror("Invalid include statement");
    }
    ;

string:
    STRING_LITERAL {
        $$ = $1;
    }
    ;

%% 

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}
