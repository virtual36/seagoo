%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "seagoo.h"

    #define MAX_INPUT_FILES 1024
    #define MAX_FILENAME_LENGTH 512

    char buffer[113];
    int buffer_empty_top = 0;

    char *input_file_queue[MAX_INPUT_FILES];
    int input_file_count = 0;

    char *done_list[MAX_INPUT_FILES];
    int done_list_count = 0;

    void add_new_file(const char * const name);
    int next_file(void);
%}

ib  \"|\<
ie  \"|\>

%x IN_NEW_INCLUDE

%option noyywrap

%%
    yyin = NULL;
    if (next_file()) {
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
    // Check if the file is already processed
    for (int i = 0; i < done_list_count; i++) {
        if (strcmp(done_list[i], name) == 0) {
            fprintf(stderr, "\033[33mSkipped file: '%s'\033[0m\n", name);
            return;
        }
    }

    // Add to the done list
    done_list[done_list_count++] = strdup(name); // Allocate memory and store the name

    // Add new file to the queue
    if (input_file_count < MAX_INPUT_FILES) {
        input_file_queue[input_file_count++] = strdup(name); // Allocate memory and store the name
        fprintf(stderr, "\033[32mNew file: '%s'\033[0m\n", name);
    } else {
        fprintf(stderr, "\033[31mInput file queue is full, cannot add: '%s'\033[0m\n", name);
    }

    // Optional: You might want to set a parent if required, depending on your logic
    const char *parent = NULL; // Replace with appropriate logic if necessary
    append_node(name, parent, DEFAULT); // Assuming DEFAULT is defined in seagoo.h
}

int next_file(void) {
    while (!yyin) {
        if (input_file_count == 0) {
            return 1; // No more files to process
        }

        yyin = fopen(input_file_queue[0], "r");
        if (!yyin) {
            perror(input_file_queue[0]);
            free(input_file_queue[0]); // Free the allocated memory
            for (int i = 0; i < input_file_count - 1; i++) {
                input_file_queue[i] = input_file_queue[i + 1];
            }
            input_file_count--;
        }
    }
    
    fprintf(stderr, "\033[34mOpening file: '%s'\033[0m\n", input_file_queue[0]);

    return 0;
}

int yywrap(void) {
    free(input_file_queue[0]); // Free the first file's name
    for (int i = 0; i < input_file_count - 1; i++) {
        input_file_queue[i] = input_file_queue[i + 1];
    }
    input_file_count--;

    yyin = NULL;
    return next_file();
}
