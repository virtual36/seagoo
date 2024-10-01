#include "seagoo.h"

/* FTW function for recursively processing nodes in file tree */
static int process_file(const char * filepath,
                        const struct stat * statbuf,
                        int typeflag) {
  if (typeflag == FTW_F || typeflag == FTW_SL) {
    if (is_non_source_file(filepath)) {
      return 0;
    }

    // add current file itself to the DB
    SourceFileNode record;
    record.filepath = strdup(filepath); // XXX: useless allocation
    if (insert_source_file(db, &record) != SQLITE_DONE) {
      fprintf(stderr, "Failed to insert source file: %s\n", record.filepath);
    }
    free(record.filepath);

    kv_push(char *, input_file_queue, strdup(filepath));
  }
  return 0;
}

/* Entry-point for indexer, accepts a codebase directory for indexing */
int index_sourcefiles(const char * directory) {
    // XXX: db not to be managed here
  const size_t sz = strlen(directory) + strlen("seagoo.db") + 2;
  char directory_full_path[sz];
  if (join_paths(directory, "seagoo.db", directory_full_path, sz)) {
    fprintf(stderr, "err: error joining paths while indexing sourcefiles\n");
  }

  if (init_db(directory_full_path) != SQLITE_OK) {
    fprintf(stderr, "Failed to initialize the database\n");
    return 1;
  }

  if (ftw(directory, process_file, 20) == -1) {
    perror("ftw");
    return 1;
  }

  yylex();

  puts("--");
  for (int i = 0; i < input_files.n; i++) {
    puts(kv_A(input_files, i));
  }

  return 0;
}

/* Frees the includes vector, if more metadata is stored then add here */
void destroy_includes_vector(includes_vector_t * vec) {
  if (vec) {
    for (size_t i = 0; i < kv_size(*vec); ++i) {
      free(kv_A(*vec, i));
    }
    kv_destroy(*vec);
    free(vec);
  }
}
