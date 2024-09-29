#include "seagoo.h"

int main(int argc, char ** argv) {
  /* +begin+ ARGUMENT PARSING */
  char * source_dir = malloc(PATH_MAX);  // the directory to parse
  if (source_dir == NULL) {
    fprintf(stderr, "err: malloc for source_dir failed\n");
    return EXIT_FAILURE;
  }
  // default to the current directory if none specifiedma
  if (getcwd(source_dir, PATH_MAX) == NULL) {
    fprintf(stderr, "err: getcwd failed\n");
    free(source_dir);
    return EXIT_FAILURE;
  }

  int option;
  while ((option = getopt(argc, argv, "d:h")) != -1) {
    switch (option) {
      case 'd':
        if (realpath(optarg, source_dir) == NULL) {
          fprintf(stderr, "err: Failed to resolve provided path: %s\n", optarg);
          free(source_dir);
          return EXIT_FAILURE;
        }
        break;  // Continue parsing for other options
      case 'h':
        fprintf(stderr, "Usage: %s [-d directory]\n", argv[0]);
        free(source_dir);
        return EXIT_SUCCESS;
      default:  // Handle unknown options
        fprintf(stderr, "err: Unknown option\n");
        free(source_dir);
        return EXIT_FAILURE;
    }
  }
  
  fprintf(stdout, "%s\n", source_dir);
  /* +end+ ARGUMENT PARSING */

  /* +begin+ CONFIGURATION FILE HANDLING */
  if (create_default_config_directory()) {
    return EXIT_FAILURE;
  }
  printf("create default config directory");
  const char * home_dir = getenv("HOME");
  if (home_dir == NULL) {
    fprintf(stderr, "err: $HOME environment variable is not set.\n");
    return EXIT_FAILURE;
  }
  char config_file[PATH_MAX];
  snprintf(config_file, sizeof(config_file), "%s/.config/seagoo/%s", home_dir,
           CONFIG_FILENAME);
  printf("%s\n", config_file);

  config_t loaded_cfg;
  load_config(config_file, &loaded_cfg);
  /* -end- CONFIGURATION FILE HANDLING */

  /* +begin+ CODEBASE INDEXING */
  index_sourcefiles(source_dir);
  /* -end- CODEBASE INDEXING */

  return EXIT_SUCCESS;
}
