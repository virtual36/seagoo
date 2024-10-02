#include "seagoo.h"

extern char source_dir[PATH_MAX];

int parse_arguments(int argc, char ** argv) {
	int option;
	while ((option = getopt(argc, argv, "d:h")) != -1) {
		switch (option) {
			case 'd':
				if (realpath(optarg, source_dir) == NULL) {
					fprintf(stderr, "err: Failed to resolve provided path: %s\n", optarg);
					return 1;
				}
				break;
			case 'h':
				printf("Usage: %s [-d directory]\n", argv[0]);
				exit(0);
			default:
				fprintf(stderr, "err: Unknown option\n");
				return 1;
		}
	}
	return 0;
}

