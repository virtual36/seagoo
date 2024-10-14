#ifndef SEAGOO_H
#define SEAGOO_H

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <ftw.h>
#include <libgen.h>
#include <limits.h>
#include <sqlite3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <magic.h>

#include "khash.h"
#include "kvec.h"
#include "log.h"
#include "ini.h"

#include "circular_queue.h"

/* +begin+ CONFIGURATION FILE HANDLING */
#define CONFIG_FILENAME "seagoo.cfg"

int load_config(const char * filename, ini_t ** cfg);
int store_config(const char * filename, const ini_t * cfg);
int write_default_config(const char * filename);
int create_default_config_directory(void);
#define get_config_value(key, cfg, out_value) ini_sget(cfg, NULL, key, NULL, out_value)
/* -end- CONFIGURATION FILE HANDLING */

/* +begin+ SOURCEFILE INDEXING */
typedef enum {
	DEFAULT,
	INTERFACE,
	SYSTEM,
} node_t;
extern int yylex();
extern int yyparse();

typedef struct {
	char * filepath;
} SourceFileNode;

int index_sourcefiles(const char * directory);
int parse_include_filepaths(const char * filepath);

extern sqlite3 * db;

int init_db(const char * db_filepath);
int create_tables(sqlite3 * db);
int insert_source_file(sqlite3 * db, const SourceFileNode * record);
int insert_include(sqlite3 * db, int source_file_id, char * included_filepath);
int get_source_file_id(sqlite3 * db, char * filepath);
int close_db(sqlite3 * db);

typedef kvec_t(char *) includes_vector_t;
includes_vector_t * lookup_includes(sqlite3 * db, const char * filepath);
void destroy_includes_vector(includes_vector_t * vec);

// used by lexer
typedef kvec_t(char *) string_vector;
extern string_vector input_file_queue; // XXX: the name is kinda misleading
extern string_vector input_files;
/* -end- SOURCEFILE INDEXING */

/* +begin+ SYNTAX TREE CONSTRUCTION */
int tbtraverse(const char * tbcode);
/* -end- SYNTAX TREE CONSTRUCTION */

/* +begin+ UTILITIES */
// XXX: explain to me why we need this
#define PATH_SEPARATOR "/"
int is_non_source_file(const char * filepath);
int join_paths(const char * left,
			   const char * right,
			   char * out,
			   size_t out_size);
char * header_name_to_path(const char * const header_name);

// this is retarded
int parse_arguments(int argc, char ** argv);
/* -end- UTILITIES */

/* +begin+ trigram search helpers */
// Data structure to hold a list of files
typedef struct FileListNode {
    char *filename;
    struct FileListNode *next;
} FileListNode;

// Data structure to hold trigram index entries
typedef struct TrigramEntry {
    uint32_t trigram;
    FileListNode *file_list;
} TrigramEntry;

// Data structure to hold the index of trigrams
typedef struct TrigramIndex {
    TrigramEntry *entries;
    size_t count;
    size_t capacity;
} TrigramIndex;

// Functions to build and search the trigram index
TrigramIndex *create_trigram_index(void);
void free_trigram_index(TrigramIndex *index);

int add_file_to_index(TrigramIndex *index, const char *filename);

int search_trigram_index(TrigramIndex *index, const char *query, char ***results, size_t *results_count);
/* -end- trigram search helpers */

#endif /* SEAGOO_H */
