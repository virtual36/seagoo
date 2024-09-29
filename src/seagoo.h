#ifndef SEAGOO_H
#define SEAGOO_H

#include <dirent.h>
#include <errno.h>
#include <libconfig.h>
#include <libgen.h>
#include <limits.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <ftw.h>

#include "lib/khash.h"
#include "lib/kstring.h"
#include "lib/kvec.h"

#include "circular_queue.h"

/* +begin+ CONFIGURATION FILE HANDLING */
#define CONFIG_FILENAME "seagoo.cfg"

int load_config(const char * filename, config_t * cfg);
int store_config(const char * filename, const config_t * cfg);
int write_default_config(const char * filename);
int create_default_config_directory();
/* -end- CONFIGURATION FILE HANDLING */

/* +begin+ SOURCEFILE INDEXING */
typedef enum {
    DEFAULT,
    INTERFACE,
    SYSTEM,
} node_t;
extern int yylex();

typedef struct {
  char filepath[PATH_MAX];
  char filename[FILENAME_MAX];
  unsigned char type;  // DIRENT directory type
} SourceFileNode;

int index_sourcefiles(const char * directory);
int parse_include_filepaths(const char * filepath);

#define CREATE_TABLES_SQL                    \
  "CREATE TABLE IF NOT EXISTS SourceFiles (" \
  "id INTEGER PRIMARY KEY AUTOINCREMENT,"    \
  "filepath TEXT NOT NULL UNIQUE,"           \
  "filename TEXT NOT NULL,"                  \
  "type INTEGER NOT NULL);"

#define INSERT_SOURCEFILE_SQL \
  "INSERT INTO SourceFiles (filepath, filename, type) VALUES (?, ?, ?);"

int init_db(const char * db_filepath);
int create_tables(sqlite3 * db);
int insert_source_file(sqlite3 * db, const SourceFileNode * record);
int close_db(sqlite3 * db);
/* -end- SOURCEFILE INDEXING */

/* +begin+ SYNTAX TREE CONSTRUCTION */
int tbtraverse(const char * const tbcode);
/* -end- SYNTAX TREE CONSTRUCTION */

#endif /* SEAGOO_H */
