#include <libconfig.h>
#include "seagoo.h"

/* global db ptr for lex parser, does this put us on a list? */
// TODO there must be another better way to parse info into Yacc
sqlite3 * db = NULL;
char * current_file_path = NULL;

int main(int argc, char ** argv) {
  /* +begin+ ARGUMENT PARSING */
  char source_dir[PATH_MAX];  // the directory to parse
  // default to the current directory if none specified
  if (getcwd(source_dir, PATH_MAX) == NULL) {
    fprintf(stderr, "err: getcwd failed\n");
    return EXIT_FAILURE;
  }

  int option;
  while ((option = getopt(argc, argv, "d:h")) != -1) {
    switch (option) {
      case 'd':
        if (realpath(optarg, source_dir) == NULL) {
          fprintf(stderr, "err: Failed to resolve provided path: %s\n", optarg);
          return EXIT_FAILURE;
        }
        break;
      case 'h':
        fprintf(stderr, "Usage: %s [-d directory]\n", argv[0]);
        return EXIT_SUCCESS;
      default:
        fprintf(stderr, "err: Unknown option\n");
        return EXIT_FAILURE;
    }
  }
  /* +end+ ARGUMENT PARSING */

  /* +begin+ CONFIGURATION FILE HANDLING */
  if (create_default_config_directory()) {
    return EXIT_FAILURE;
  }
  const char * home_dir = getenv("HOME");
  if (home_dir == NULL) {
    fprintf(stderr, "err: $HOME environment variable is not set.\n");
    return EXIT_FAILURE;
  }
  char config_file[PATH_MAX];
  snprintf(config_file, sizeof(config_file), "%s/.config/seagoo/%s", home_dir,
           CONFIG_FILENAME);

  config_t cfg;
  load_config(config_file, &cfg);
  /* -end- CONFIGURATION FILE HANDLING */

  /* +begin+ CODEBASE INDEXING */
  index_sourcefiles(source_dir);
  /* -end- CODEBASE INDEXING */

  /* +begin+ CLEANUP */
  close_db(db);
  config_destroy(&cfg);
  /* -end- CLEANUP */

  return EXIT_SUCCESS;
}
