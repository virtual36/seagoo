%{
#include "include_parser.tab.h"
#include <string.h>
%}

%%
[ \t\n]+                     ; // Ignore whitespace
"//".*                       ; // Ignore single-line comments
"/*".*                       ; // Ignore multi-line comments
"#include"                   { return INCLUDE; }
"\"[^\"]*\""                 { yylval.str = strdup(yytext); return STRING_LITERAL; }
"<[^>]*>"                    { yylval.str = strdup(yytext); return STRING_LITERAL; }
.                            { /* Ignore any other characters */ }

%% 

int yywrap() {
    return 1;
}
