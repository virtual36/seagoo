#include "seagoo.h"

/* Entry-point for indexer, accepts a codebase directory for indexing */
int index_sourcefiles(const char * directory) {
  DIR * dir;
  struct dirent * entry;

  sqlite3 * db;
  if (init_db("source_files.db") != SQLITE_OK) {
    fprintf(stderr, "Failed to initialize the database\n");
    return 1;
  }

  if ((dir = opendir(directory)) == NULL) {
    perror("opendir");
    return 1;
  }

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG || entry->d_type == DT_LNK) {
      // Process regular files or symlinks
      char fullpath[PATH_MAX];
      snprintf(fullpath, PATH_MAX, "%s/%s", directory, entry->d_name);

      // Parse the file to gather includes
      parse_include_filepaths(fullpath);
    }
  }

  closedir(dir);
  close_db(db);
  return 0;
}

/* Uses depgra's Lex parser to grab includes from file and add them to DB */
int parse_include_filepaths(const char * filepath) {
  FILE * file = fopen(filepath, "r");
  if (!file) {
    perror("Failed to open file");
    return 1;
  }

  extern FILE * yyin;
  yyin = file;

  yylex();

  fclose(file);
  return 0;
}

/* Initialises index database at a specific filepath with required tables */
int init_db(const char * db_filepath) {
  sqlite3 * db;
  int rc = sqlite3_open(db_filepath, &db);

  if (rc) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    return rc;
  }

  return create_tables(db);
}

/* Creates tables in the SQLite database */
int create_tables(sqlite3 * db) {
  char * errmsg = 0;
  int rc = sqlite3_exec(db, CREATE_TABLES_SQL, 0, 0, &errmsg);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return rc;
  }

  return SQLITE_OK;
}

/* Offloads a SourceFileNode in-memory graph node into the database */
int insert_source_file(sqlite3 * db, const SourceFileNode * record) {
  sqlite3_stmt * stmt;
  int rc = sqlite3_prepare_v2(db, INSERT_SOURCEFILE_SQL, -1, &stmt, NULL);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return rc;
  }

  sqlite3_bind_text(stmt, 1, record->filepath, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, record->filename, -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 3, record->type);

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
  }

  sqlite3_finalize(stmt);
  return rc;
}

/* Closes the database */
int close_db(sqlite3 * db) {
  return sqlite3_close(db);
}