#include "seagoo.h"

#define READ_SAMPLE_SIZE 512

/* XXX: this file is terrible;
 * might as well call it "Toad Butt Sex";
 * thats just as helpful
 */

/* Check if something is a non-parsable file without extension checks */
// XXX: mimetypes exist
int is_binary_file(const char * filepath) {
  FILE * file = fopen(filepath, "rb");
  if (!file)
    return 1;

  unsigned char buffer[READ_SAMPLE_SIZE];
  size_t bytes_read = fread(buffer, 1, READ_SAMPLE_SIZE, file);
  fclose(file);

  for (size_t i = 0; i < bytes_read; ++i) {
    if (!isprint(buffer[i]) && !isspace(buffer[i])) {
      return 1; /* binary file*/
    }
  }

  return 0; /* text file */
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
