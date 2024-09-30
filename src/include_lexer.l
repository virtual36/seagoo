%{
    #include <stdio.h>
    #include <string.h>

    #include "seagoo.h"
    #include "kvec.h"

    extern void append_node(const char * const name, const char * const parent, const node_t type);

    char buffer[PATH_MAX];
    int  buffer_empty_top = 0;

    string_vector input_file_queue;
    string_vector input_files;
    string_vector input_error_list;

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
    for (int i = 0; i < input_error_list.n; i++) {
        if (!strcmp(name, kv_A(input_error_list, i))) {
            goto skip;
        }
    }

    // XXX: if header_name_to_path() is fixed, no NULL check is needed
    char * new_name = header_name_to_path(name);
    if (!new_name) {
        kv_push(char *, input_error_list, strdup(name));
        return;
    }

    for (int i = 0; i < input_files.n; i++) {
        if (!strcmp(new_name, kv_A(input_files, i))) {
            goto skip;
        }
    }

    for (int i = 0; i < input_file_queue.n; i++) {
        if (!strcmp(new_name, kv_A(input_file_queue, i))) {
            goto skip;
        }
    }

    fprintf(stderr, "\033[32mNew file: '%s'\033[0m\n", new_name);
    kv_push(char *, input_file_queue, new_name);

    return;

  skip: // XXX debug code; just return instead
    fprintf(stderr, "\033[33mSkipped file: '%s'\033[0m\n", new_name);
}

int yywrap(void) {
    char * filename;

    yyin = NULL;
    while (!yyin) {
        if (input_file_queue.n == 0) { return 1; }

        filename = kv_A(input_file_queue, input_file_queue.n-1);
        if (!filename) { goto end; }

        yyin = fopen(filename, "r");
        if (!yyin) {
            kv_push(char *, input_error_list, filename);
            perror(filename);
            free(filename);
        }
      end:
        (void)kv_pop(input_file_queue);
    }
    
    fprintf(stderr, "\033[34mOpening file: '%s'\033[0m\n", filename);

    kv_push(char *, input_files, strdup(filename));

    return 0;
}
