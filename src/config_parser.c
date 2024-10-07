#include "seagoo.h"

/* Load configuration from a file */
int load_config(const char * filename, ini_t ** cfg) {
	if (access(filename, F_OK) != 0) {
		log_error("file '%s' doesn't exist\n", filename);
		return 1;
	}

	if ((*cfg = ini_load(filename)) == NULL) {
		log_error("file '%s' couldn't be loaded\n", filename);
		return 1;
	}
	
	return 0;
}

/* Store configuration to a file */
int store_config(const char * filename, const ini_t * cfg) {
	log_info("we don't do that here");
	return 0;
}

/* Write a default template configuration file */
int write_default_config(const char * filename) {
	log_info("we don't do that here");
	return 0;
}

/* Create an empty configuration directory with defaults if it doesn't exist */
int create_default_config_directory(void) {
	const char * home_dir = getenv("HOME");
	if (home_dir == NULL) {
		log_error("$HOME environment variable is not set.\n");
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
		if (mkdir(config_dir, S_IRWXU) != 0) {
			log_error("problem creating config directory '%s': %s\n",
					config_dir, strerror(errno));
			return EXIT_FAILURE;
		}
		if (write_default_config(config_file)) {
			return EXIT_FAILURE;
		}
	} else if (!S_ISDIR(st.st_mode)) {
		log_error("'%s' exists but is not a directory\n", config_dir);
		return EXIT_SUCCESS;
	}

	return EXIT_SUCCESS;
}

