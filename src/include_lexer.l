%{
    #include <stdio.h>
    #include <string.h>

    #include "seagoo.h"
    #include "kvec.h"

    extern void append_node(const char * const name, const char * const parent, const node_t type);

    char buffer[PATH_MAX];
    int  buffer_empty_top = 0;

    string_vector input_file_queue;
    string_vector done_list;

    void add_new_file(const char * const name);
%}

ib  \"|\<
ie  \"|\>

%x IN_NEW_INCLUDE

%option nodefault
%%
    yyin = NULL;
    if(yywrap()) {
        return 1;
    }

^[[:space:]]*\#[[:space:]]*include[[:space:]]*{ib} {
        BEGIN IN_NEW_INCLUDE;
        buffer[buffer_empty_top++] = yytext[yyleng-1];
    }

<IN_NEW_INCLUDE>{
{ie} {
         BEGIN INITIAL;

         buffer[buffer_empty_top++] = yytext[0];
         buffer[buffer_empty_top]   = '\0';

         add_new_file(buffer);

         buffer_empty_top = 0;
     }
.    {
         buffer[buffer_empty_top++] = yytext[0];
     }
\n   { BEGIN INITIAL; }
}

.|\n { ; }
%%

void add_new_file(const char * const name) {
    for (int i = 0; i < done_list.n; i++) {
        if (!strcmp(name, kv_A(done_list, i))) {
            goto skip;
        }
    }

    for (int i = 0; i < input_file_queue.n; i++) {
        if (!strcmp(name, kv_A(input_file_queue, i))) {
            goto skip;
        }
    }

    fprintf(stderr, "\033[32mNew file: '%s'\033[0m\n", name);
    kv_push(char *, input_file_queue, strdup(name));

    return;

  skip: // XXX debug code; just return instead
    fprintf(stderr, "\033[33mSkipped file: '%s'\033[0m\n", name);
}

int yywrap(void) {
    char * filename;

    yyin = NULL;
    while (!yyin) {
        if (input_file_queue.n == 0) { return 1; }

        filename = kv_A(input_file_queue, input_file_queue.n-1);
        yyin = fopen(filename, "r");
        if (!yyin) {
            perror(filename);
            free(filename);
        }
        (void)kv_pop(input_file_queue);
    }
    
    fprintf(stderr, "\033[34mOpening file: '%s'\033[0m\n", filename);

    kv_push(char *, done_list, strdup(filename));

    return 0;
}
