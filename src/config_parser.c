#include "seagoo.h"

/* Load configuration from a file */
int load_config(const char * filename, config_t * cfg) {
  config_init(cfg);

  if (!config_read_file(cfg, filename)) {
    fprintf(stderr, "err: error reading the config file: %s\n",
            config_error_text(cfg));
    config_destroy(cfg);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/* Store configuration to a file */
int store_config(const char * filename, const config_t * cfg) {
  FILE * file = fopen(filename, "w");
  if (!file) {
    fprintf(stderr, "err: problem opening config file for writing\n");
    return EXIT_FAILURE;
  }

  config_write(cfg, file);
  fclose(file);
  return EXIT_SUCCESS;
}

/* Write a default template configuration file */
int write_default_config(const char * filename) {
  config_t cfg;
  config_init(&cfg);

  config_setting_t * root = config_root_setting(&cfg);

  // Add default settings to the config_t object
  config_setting_t * setting =
      config_setting_add(root, "grep_binary", CONFIG_TYPE_STRING);
  config_setting_set_string(setting, "/usr/bin/rg"); /* just example */

  setting = config_setting_add(root, "sqlite_db", CONFIG_TYPE_STRING);
  config_setting_set_string(setting, "~/seagoo.db"); /* just example */

  setting = config_setting_add(root, "num_results_to_list", CONFIG_TYPE_INT);
  config_setting_set_int(setting, 5);

  setting = config_setting_add(root, "graph_pruning_distance", CONFIG_TYPE_INT);
  config_setting_set_int(setting, 15);

  setting = config_setting_add(root, "parse_comments", CONFIG_TYPE_BOOL);
  config_setting_set_bool(setting, 0);

  // list of regex patterns for files to whitelist and blacklist
  setting = config_setting_add(root, "regex_blacklist_files", CONFIG_TYPE_LIST);

  // initialise regex_whitelist_files with allowed file extensions
  setting = config_setting_add(root, "regex_whitelist_files", CONFIG_TYPE_LIST);

  // Adding allowed file extensions to the whitelist
  config_setting_t * whitelist =
      config_setting_add(root, "regex_whitelist_files", CONFIG_TYPE_LIST);
  // Adding allowed file extensions to the whitelist
  setting = config_setting_add(whitelist, NULL, CONFIG_TYPE_STRING);
  config_setting_set_string(setting, "*.c");
  setting = config_setting_add(whitelist, NULL, CONFIG_TYPE_STRING);
  config_setting_set_string(setting, "*.h");
  setting = config_setting_add(whitelist, NULL, CONFIG_TYPE_STRING);
  config_setting_set_string(setting, "*.hpp");
  setting = config_setting_add(whitelist, NULL, CONFIG_TYPE_STRING);
  config_setting_set_string(setting, "*.cc");
  setting = config_setting_add(whitelist, NULL, CONFIG_TYPE_STRING);
  config_setting_set_string(setting, "*.cpp");

  if (store_config(filename, &cfg) != EXIT_SUCCESS) {
    config_destroy(&cfg);
    return EXIT_FAILURE;
  }

  config_destroy(&cfg);
  return EXIT_SUCCESS;
}

/* Create an empty configuration directory with defaults if it doesn't exist */
int create_default_config_directory() {
  const char * home_dir = getenv("HOME");
  if (home_dir == NULL) {
    fprintf(stderr, "err: $HOME environment variable is not set.\n");
    return EXIT_FAILURE;
  }

  // Construct the full path: $HOME/.config/seagoo
  char config_dir[PATH_MAX - sizeof(CONFIG_FILENAME) - 1];
  snprintf(config_dir, sizeof(config_dir), "%s/.config/seagoo", home_dir);
  char config_file[PATH_MAX];
  snprintf(config_file, sizeof(config_file), "%s/%s", config_dir,
           CONFIG_FILENAME);

  struct stat st;

  if (stat(config_dir, &st) == -1) {
    if (mkdir(config_dir, 0700) != 0) {
      fprintf(stderr, "err: problem creating config directory '%s': %s\n",
              config_dir, strerror(errno));
      return EXIT_FAILURE;
    }
    if (write_default_config(config_file)) {
      return EXIT_FAILURE;
    }
  } else if (!S_ISDIR(st.st_mode)) {
    fprintf(stderr, "err: '%s' exists but is not a directory\n", config_dir);
    return EXIT_SUCCESS;
  }

  return EXIT_SUCCESS;
}
