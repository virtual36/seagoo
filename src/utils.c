#include "seagoo.h"

#define READ_SAMPLE_SIZE 512

/* XXX: this file is terrible;
 * might as well call it "Toad Butt Sex";
 * thats just as helpful
 */

/* Check if something is a non-parsable file without extension checks */
    /* TODO: support in the config adding a list of regex patterns for
     * a whitelist and blacklist. This is so that users can indicate to
     * us what files they want us to index.
     */
static magic_t mimetype_reader = NULL; // XXX: it does very strange things with memory

int is_non_source_file(const char * filepath) {
	const char * const prefix_blacklist[] = {
		"application",
	};

	if (!mimetype_reader) {
		mimetype_reader = magic_open(MAGIC_MIME_TYPE);
	}

	if (magic_load(mimetype_reader, NULL) == -1) { return 1; }

	// NOTE: It *is* a const char *; do not try to free it; copy it if you have to
	const char * mime = magic_file(mimetype_reader, filepath);
	if (mime == NULL) { return 1; }

	//magic_close(mimetype_reader); // XXX

	for (int i = 0; i < (sizeof(prefix_blacklist) / sizeof(char *)); i++) {
		if (!memcmp(mime, prefix_blacklist[i], strlen(prefix_blacklist[i]))) {
			log_debug("'%s' was not a source file (%s)", filepath, mime);
			return 1;
		}
	}

	return 0;
}

int join_paths(const char * left,
			   const char * right,
			   char * out,
			   size_t out_size) {
	// TODO: move to this function
	if (!left || !right || !out) {
		return 1; /* null pointer */
	}

	size_t left_len = strlen(left);
	size_t right_len = strlen(right);

	size_t required_size = left_len + right_len + 2;

	if (required_size > out_size) {
		return 1; /* buffer too small */
	}

	strcpy(out, left);

	// add seperator if left does not end with PATH_SEP already
	if (left[left_len - 1] != '/') {
		strcat(out, PATH_SEPARATOR);
	}

	strcat(out, right);

	return 0;
}

/* XXX: this function should search in the include path;
 *       the current implentation is a quick hack
 */
char * header_name_to_path(const char * const header_name) {
	const char source_dir[] = "src/";
	char * r;

	if (header_name[0] == '"') {
		int l = strlen(header_name);
		r = malloc(l + sizeof(source_dir));
		memcpy(r, source_dir, sizeof(source_dir)-1);
		memcpy(r + (sizeof(source_dir)-1), header_name + 1, l-2);
		r[l + sizeof(source_dir) - 2 - 1] = '\00';
	} else
	if (header_name[0] == '<') {
		r = NULL;
	} else {
		r = strdup(header_name);
	}

	return r;
}

