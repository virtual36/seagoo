#include "seagoo.h"

int main(int argc, char ** argv) {
  /* +begin+ ARGUMENT PARSING */
  char * source_dir = NULL;  // the directory to parse
  int option;
  getcwd(source_dir, PATH_MAX);  // default to current directory
  while ((option = getopt(argc, argv, "d:")) != -1) {
    switch (option) {
      case 'd':
        if (!realpath(source_dir, optarg)) {
          fprintf(stderr, "err: Failed to resolve provided path\n");
        }
        break;
      case 'h':
        fprintf(stderr, "Usage: %s [-d directory]\n", argv[0]);
        return EXIT_FAILURE;
    }
  }
  /* +end+ ARGUMENT PARSING */

  /* +begin+ CONFIGURATION FILE HANDLING */
  if (!create_default_config_directory()) {
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

  config_t loaded_cfg;
  load_config(config_file, &loaded_cfg);
  /* -end- CONFIGURATION FILE HANDLING */

  /* +begin+ CODEBASE INDEXING */
  index_sourcefiles(source_dir);
  /* -end- CODEBASE INDEXING */

  return EXIT_SUCCESS;
}
