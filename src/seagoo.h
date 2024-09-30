#ifndef SEAGOO_H
#define SEAGOO_H

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <ftw.h>
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
int get_config_value(const char * key, const config_t * cfg, void * out_value);
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
extern char * current_file_path;

#define CREATE_TABLES_SQL                    \
  "CREATE TABLE IF NOT EXISTS SourceFiles (" \
  "id INTEGER PRIMARY KEY AUTOINCREMENT,"    \
  "filepath TEXT NOT NULL UNIQUE);"

#define INSERT_SOURCEFILE_SQL \
  "INSERT OR IGNORE INTO SourceFiles (filepath) VALUES (?);"

// Define the Includes table
#define CREATE_INCLUDES_TABLE_SQL                            \
  "CREATE TABLE IF NOT EXISTS Includes ("                    \
  "id INTEGER PRIMARY KEY AUTOINCREMENT,"                    \
  "source_file_id INTEGER,"                                  \
  "included_file_id INTEGER,"                                \
  "FOREIGN KEY (source_file_id) REFERENCES SourceFiles(id)," \
  "FOREIGN KEY (included_file_id) REFERENCES SourceFiles(id));"

// Insert statement for the Includes table
#define INSERT_INCLUDE_SQL                                                    \
  "INSERT OR IGNORE INTO Includes (source_file_id, included_file_id) VALUES " \
  "(?, ?);"

// Query to lookup includes for a given source file
#define LOOKUP_INCLUDES_SQL                                              \
  "SELECT sf_included.filepath "                                         \
  "FROM Includes i "                                                     \
  "JOIN SourceFiles sf ON i.source_file_id = sf.id "                     \
  "JOIN SourceFiles sf_included ON i.included_file_id = sf_included.id " \
  "WHERE sf.filepath = ?;"

int init_db(const char * db_filepath);
int create_tables(sqlite3 * db);
int insert_source_file(sqlite3 * db, const SourceFileNode * record);
int insert_include(sqlite3 * db, int source_file_id, char * included_filepath);
int get_source_file_id(sqlite3 * db, char * filepath);
int close_db(sqlite3 * db);

typedef kvec_t(char *) includes_vector_t;
includes_vector_t * lookup_includes(sqlite3 * db, const char * filepath);
void destroy_includes_vector(includes_vector_t * vec);
/* -end- SOURCEFILE INDEXING */

/* +begin+ SYNTAX TREE CONSTRUCTION */
int tbtraverse(const char * tbcode);
/* -end- SYNTAX TREE CONSTRUCTION */

/* +begin+ UTILITIES */
#define PATH_SEPARATOR "/"
int is_binary_file(const char * filepath);
int join_paths(const char * left,
               const char * right,
               char * out,
               size_t out_size);
char * header_name_to_path(const char * const header_name);
/* -end- UTILITIES */

typedef kvec_t(char *) string_vector;
extern string_vector input_file_queue; // XXX: the name is kinda misleading

#endif /* SEAGOO_H */
