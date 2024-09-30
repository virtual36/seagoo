%{
#include "include_parser.tab.h"
#include "seagoo.h"
#include <string.h>
#include <stdio.h>

extern int yylineno;
%}

%x IN_COMMENT
%%

[ \t\n]+                      ; 

"//".*$                       ; 

"/*"        { BEGIN(IN_COMMENT); }  

<IN_COMMENT>"*/" { BEGIN(INITIAL); }  

<IN_COMMENT>.|\n  ; 

"#include"[ \t]+"\"[^\"]*\"" {
                                printf("[Lexer] Matched #include with double-quoted string: %s at line %d\n", yytext, yylineno);
                                yylval.str = strndup(yytext + 10, yyleng - 11);
                                printf("sendiing %s\n", yylval.str);
                                return INCLUDE; 
                              }

"#include"[ \t]+\<[^\>]*\> {
                                printf("[Lexer] Matched #include with angle-bracketed string: %s at line %d\n", yytext, yylineno);
                                yylval.str = strndup(yytext + 10, yyleng - 11);
                                printf("sendiing %s\n", yylval.str);
                                return INCLUDE; 
                              }

.                             { 
                                /* ignore unknown char */
                              }

%%

int yywrap() {
    return 1;
}
