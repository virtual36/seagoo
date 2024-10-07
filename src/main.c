#include <libconfig.h>
#include "seagoo.h"

char source_dir[PATH_MAX];  // the directory to parse

int main(int argc, char ** argv) {

	printf("%d\n", 0700);
	return 0;
	/* +begin+ ARGUMENT PARSING */
	if (parse_arguments(argc, argv)) {
		return EXIT_FAILURE;
	}

	if (!source_dir[0]) {
		if (!getcwd(source_dir, PATH_MAX)) {
			log_error("getcwd failed\n");
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
		log_error("$HOME environment variable is not set.\n");
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

