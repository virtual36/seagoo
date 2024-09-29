/* TODO: once this gets big enough, we can move to multiple headers
 *       rather than just one ginormous header file */

#ifndef SEAGOO_H
#define SEAGOO_H

#include <dirent.h>
#include <errno.h>
#include <libconfig.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "lib/khash.h"
#include "lib/kstring.h"
#include "lib/kvec.h"

#include "circular_queue.h"

/* +begin+ CONFIGURATION FILE HANDLING */
#define CONFIG_FILENAME "seagoo.cfg"

int load_config(const char * filename, config_t * cfg);
int store_config(const char * filename, const config_t * cfg);
int write_default_config(const char * filename);
int create_default_config_directory();
/* -end- CONFIGURATION FILE HANDLING */

/* +begin+ SOURCEFILE INDEXING */
#define MAX_INCLUDES_TO_PARSE 1024
#define MAX_INCLUDE_LENGTH 512
#define INCLUDE_LINE_LENGTH 1024

typedef struct {
  char * include_filepaths[MAX_INCLUDES_TO_PARSE];
  char filepath[PATH_MAX];
  char filename[FILENAME_MAX];
  unsigned char type;  // DIRENT directory type
} SourceFileNode;

int index_sourcefiles(const char * directory);
int parse_include_filepaths(char * filepath, char ** include_filepaths);
int tbtraverse(const char * const tbcode);
/* -end- SOURCEFILE INDEXING */

#endif /* SEAGOO_H */
