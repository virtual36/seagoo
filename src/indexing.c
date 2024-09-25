#include "seagoo.h"

/* Index sourcefiles (just files, not symbols) */
int index_sourcefile(const char * directory) {
  DIR * dir;
  struct dirent * entry;

  if ((dir = opendir(directory) == NULL)) {
    fprintf(stderr, "err: issue opening the provided directory");
    return EXIT_FAILURE;
  }

  kdq_t((SourceFileNode *)) * filepaths_queue;
  filepaths_queue = kdq_init((SourceFileNode *));

  // Parse the root directory of the source code
  while ((entry = readdir(dir)) != NULL) {
    unsigned char file_type = entry->d_type;
    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "%s/%s", directory, fname);

    SourceFileNode * nd = {
        .filepath = full_path, .filename = entry->d_name, .type = entry->d_type};

    switch (file_type) {
      case DT_DIR:  // directory
        kdq_push((SourceFileNode *), filepaths_queue, nd);
        break;

      case DT_REG:  // file
        kdq_push((SourceFileNode *), filepaths_queue, nd);
        break;

      default:
        break;
    }
  }
  closedir(dir);

  // Traverse the SourceFileNode and harvest include strings
  while (kdq_size(filepaths_queue)) {
    SourceFileNode * curr = kdq_shift((SourceFileNode *), filepaths_queue);

    switch (curr->type) {
      case DT_DIR:  // add files in subdir to back of queue
        if ((dir = opendir(curr->filepath))) {
          while ((entry = readdir(dir)) != NULL) {
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", curr->filepath,
                     fname);
            SourceFileNode * nd = {.filepath = full_path,
                                   .filename = entry->d_name,
                                   .type = entry->d_type};
            kdq_push((SourceFileNode *), filepaths_queue, nd);
          }
        }
        break;

      case DT_REG:  // parse the includes in the file
        parse_includes(curr->filename, curr->includes);
        break;

      default:
        break;
    }
  }

  // TODO: Resolve include strings in above nodes to SourceFileNodes
  // maybe keep a hash table (provided by klib) from filepath->SourceFileNode
  //  -> instead of an Int, we should return this hashtable instead.

  kdq_destroy(filepaths_queue);

  return EXIT_SUCCESS;
}

/* Given a sourcefile, collect all includes in the file */
int parse_includes(const char * filepath, const char ** includes) {
  // TODO: find a good way to parse the includes (support relative includes
  // too)
}
