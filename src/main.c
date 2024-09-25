#include "seagoo.h"

int main(int argc, char ** argv) {
  char * source_dir = NULL;  // the directory to parse
  int option;
  source_dir = getcwd();  // default to current directory
  while ((option = getopt(argc, argv, "d:")) != -1) {
    switch (option) {
      case 'd':
        source_dir = realpath(optarg);
        break;
      case 'h':
        fprintf(stderr, "Usage: %s [-d directory]\n", argv[0]);
        return 1;
    }
  }

  /* +begin+ CONFIGURATION FILE HANDLING */
  if (!create_default_config_directory()) {
    return 0;
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
  index_codebase(source_dir);
  /* -end- CODEBASE INDEXING */

  return 0;
}
