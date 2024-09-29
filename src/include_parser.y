%{
#include "seagoo.h"

void yyerror(const char *s);
int yylex(void);

extern sqlite3 *db;
extern char * current_file_path;
extern int insert_include(sqlite3 * db,
                          int source_file_id,
                          char * included_filepath);
extern int get_source_file_id(sqlite3 * db, char * filepath);
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
        // printf("Found include: %s\n", $2);

        int key = get_source_file_id(db, current_file_path);
        if (key == -1) {
            goto frees;
        }

        SourceFileNode record;
        record.filepath = strdup(current_file_path);

        char * include_file = strdup($2);
        if (!include_file) {
            fprintf(stderr, "Memory allocation failed for include_file\n");
            YYABORT;
        }

        if (insert_include(db, key, include_file) != SQLITE_OK) {
            fprintf(stderr, "Failed to insert include: %s\n", record.filepath);
        }

    frees:
        free(record.filepath);
        free(include_file);
        // free(yylval.str); 
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
