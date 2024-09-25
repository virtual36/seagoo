/* TODO: once this gets big enough, we can move to multiple headers
 *       rather than just one ginormous header file */

#ifndef SEAGOO_H
#define SEAGOO_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <libconfig.h>

#include "lib/kvec.h"
#include "lib/kstring.h"
#include "lib/khash.h"

/* +begin+ CONFIGURATION FILE HANDLING */
#define CONFIG_DIRECTORY ".config"
#define CONFIG_FILENAME  "seagoo.cfg"

int load_config(const char *filename, config_t *cfg);
int store_config(const char *filename, const config_t *cfg);
int write_default_config(const char *filename);

/* -end- CONFIGURATION FILE HANDLING */

#endif /* SEAGOO_H */
