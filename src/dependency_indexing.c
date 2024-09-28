#include "seagoo.h"

/* [filename.c] => [x (ptr)] -> SourceFileNode */
KHASH_MAP_INIT_STR(file_map, SourceFileNode *);

/* private members */
char * resolve_include_path(const char * current_file,
                            const char * include_path);
int process_entry(const char * full_path,
                  unsigned char file_type,
                  circular_queue * queue,
                  khash_t(file_map) * file_map);

/* Index source files (just files, not symbols), entry-point for this indexer */
int index_sourcefiles(const char * directory) {
  DIR * dir;
  struct dirent * entry;

  if ((dir = opendir(directory)) == NULL) {
    fprintf(stderr, "err: issue opening the provided directory\n");
    return EXIT_FAILURE;
  }

  circular_queue filepaths_queue;
  if (circular_queue_init(&filepaths_queue, MAX_INCLUDES_TO_PARSE) < 0) {
    fprintf(stderr, "err: failed to initialize the circular queue\n");
    return EXIT_FAILURE;
  }

  khash_t(file_map) * file_map = kh_init(file_map);
  if (!file_map) {
    fprintf(stderr, "err: failed to initialize file_map\n");
    return EXIT_FAILURE;
  }

  // Scan the directory and populate the queue
  while ((entry = readdir(dir)) != NULL) {
    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);

    if (process_entry(full_path, entry->d_type, &filepaths_queue, file_map) <
        0) {
      closedir(dir);
      return EXIT_FAILURE;
    }
  }
  closedir(dir);

  // Depth-first search the initially scanned file graph
  while (!circular_queue_is_empty(&filepaths_queue)) {
    SourceFileNode * curr;
    circular_queue_dequeue(&filepaths_queue, (void **)&curr);

    switch (curr->type) {
      case DT_DIR:  // directory
        if ((dir = opendir(curr->filepath))) {
          while ((entry = readdir(dir)) != NULL) {  // add children into queue
            char full_path[PATH_MAX + FILENAME_MAX + 1];
            snprintf(full_path, sizeof(full_path), "%s/%s", curr->filepath,
                     entry->d_name);

            if (process_entry(full_path, entry->d_type, &filepaths_queue,
                              file_map) < 0) {
              closedir(dir);
              return EXIT_FAILURE;
            }
          }
          closedir(dir);
        }
        break;

      case DT_REG:  // regular file
        if (parse_include_filepaths(curr->filepath, curr->include_filepaths) ==
            0) {
          for (size_t i = 0; i < MAX_INCLUDES_TO_PARSE; ++i) {
            if (curr->include_filepaths[i]) {
              if (process_entry(curr->include_filepaths[i], DT_REG,
                                &filepaths_queue, file_map) < 0) {
                return EXIT_FAILURE;
              }
            }
          }
        }
        break;

      default:
        break;
    }

    free(curr);
  }

  circular_queue_destroy(&filepaths_queue);

  // Clean up file_map
  for (khint_t k = kh_begin(file_map); k != kh_end(file_map); ++k) {
    if (kh_exist(file_map, k)) {
      free((char *)kh_key(file_map, k));  // free the key (filepath)
      free(kh_value(file_map, k));        // free the value (SourceFileNode *)
    }
  }
  kh_destroy(file_map, file_map);

  return EXIT_SUCCESS;
}

/* Given the current file path (where include was found), find absolute path */
char * resolve_include_path(const char * current_file,
                            const char * include_path) {
  char resolved_path[PATH_MAX];

  // check if it's already an absolute path
  if (include_path[0] == '/') {
    if (realpath(include_path, resolved_path) != NULL) {
      return strdup(resolved_path);
    } else {
      fprintf(stderr, "err: failed to resolve absolute path for %s\n",
              include_path);
      return NULL;
    }
  }

  // for relative include paths ("file.h"), resolve relative to current dir
  if (include_path[0] == '"' || include_path[0] == '<') {
    // get dir of the file
    char current_file_copy[PATH_MAX];
    strncpy(current_file_copy, current_file, sizeof(current_file_copy));
    char * current_dir = dirname(current_file_copy);
    // build relative path up to the file
    snprintf(resolved_path, sizeof(resolved_path), "%s/%s", current_dir,
             include_path);

    // resolve that path to an absolute path
    char temp_resolved_path[PATH_MAX];
    if (realpath(resolved_path, temp_resolved_path) != NULL) {
      return strdup(temp_resolved_path);
    } else {
      fprintf(stderr, "err: failed to resolve relative path for %s\n",
              include_path);
    }
  }

  // search through the standard directories for system includes
  if (include_path[0] == '<' && include_path[strlen(include_path) - 1] == '>') {
    // remove angle brackets from string
    const char * default_include_path = "/usr/include";
    char base_include_path[PATH_MAX - strlen(default_include_path) - 1];
    strncpy(base_include_path, include_path + 1, sizeof(base_include_path));
    base_include_path[strlen(base_include_path) - 1] = '\0';

    char temp_resolved_path[PATH_MAX];
    snprintf(resolved_path, sizeof(resolved_path), "%s/%s", default_include_path,
             base_include_path);
    if (realpath(resolved_path, temp_resolved_path) != NULL) {
      return strdup(temp_resolved_path);
    }
  }

  fprintf(stderr, "err: failed to resolve system include path for %s\n",
          include_path);

  return NULL;
}

/* Procedure to process a given file/directory/other (private member) */
int process_entry(const char * full_path,
                  unsigned char file_type,
                  circular_queue * queue,
                  khash_t(file_map) * file_map) {
  // Check if the file path already exists in the hashmap
  int absent;
  khint_t k = kh_put(file_map, file_map, strdup(full_path), &absent);
  if (!absent) {
    return 0;  // Already in the map, skip processing
  }

  SourceFileNode * nd = malloc(sizeof(SourceFileNode));
  if (!nd) {
    fprintf(stderr, "err: memory allocation for SourceFileNode failed\n");
    return -1;
  }

  strncpy(nd->filepath, full_path, PATH_MAX);
  nd->type = file_type;

  if (file_type == DT_DIR || file_type == DT_REG) {
    circular_queue_enqueue(queue, nd);
    kh_value(file_map, k) = nd;
  }
  return 0;
}

/* Given a sourcefile, collect all include filepaths mentioned in the file */
int parse_include_filepaths(char * filepath, char ** include_filepaths) {
  FILE * file = fopen(filepath, "r");
  if (!file) {
    fprintf(stderr, "err: could not open file %s\n", filepath);
    return EXIT_FAILURE;
  }

  char line[INCLUDE_LINE_LENGTH];
  size_t include_count = 0;

  while (fgets(line, sizeof(line), file) != NULL) {
    char * include_start = NULL;

    // search for the "#include" keyword
    if ((include_start = strstr(line, "#include")) != NULL) {
      include_start += strlen("#include");  // skip include
      while (*include_start == ' ' || *include_start == '\t') {
        include_start++;  // remove whitespace
      }

      char * start = NULL;
      char * end = NULL;

      // check if it's an angle baracket include like <stdio.h>
      if ((start = strchr(include_start, '<')) &&
          (end = strchr(include_start, '>'))) {
        start++;  // skip the first <
      } else if ((start = strchr(include_start, '"')) &&
                 (end = strrchr(include_start, '"'))) {
        // check if it's a 'my_file.h' type include
        start++;  // skip the first quote
      }

      if (start && end && include_count < MAX_INCLUDES_TO_PARSE) {
        size_t length = end - start;
        if (length >= MAX_INCLUDE_LENGTH) {
          fprintf(stderr, "err: include path too long\n");
          fclose(file);
          return EXIT_FAILURE;
        }

        include_filepaths[include_count] = malloc(length + 1);
        if (!include_filepaths[include_count]) {
          fprintf(stderr,
                  "err: memory allocation for include filepath failed\n");
          fclose(file);
          return EXIT_FAILURE;
        }

        // finally copy the resolved include path to a new string for the array
        strncpy(include_filepaths[include_count], start, length);
        include_filepaths[include_count][length] = '\0';

        // resolve this include path to an absolute path TODO: very iffy
        char * res =
            resolve_include_path(filepath, include_filepaths[include_count]);
        if (res) {
          include_filepaths[include_count] = res;
        } else {
          fprintf(stderr, "err: failed to resolve file with path %s\n",
                  include_filepaths[include_count]);
        }

        include_count++;
      }
    }
  }

  fclose(file);
  return EXIT_SUCCESS;
}
