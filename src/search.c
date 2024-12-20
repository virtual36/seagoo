#include "seagoo.h"


static uint32_t encode_trigram(const char *str) {
    return ((uint32_t)(unsigned char)str[0] << 16) |
           ((uint32_t)(unsigned char)str[1] << 8) |
           (uint32_t)(unsigned char)str[2];
}
TrigramIndex *create_trigram_index(void) {
    TrigramIndex *index = (TrigramIndex *)malloc(sizeof(TrigramIndex));
    if (!index) {
        return NULL;
    }
    index->entries = NULL;
    index->count = 0;
    index->capacity = 0;
    return index;
}
void free_trigram_index(TrigramIndex *index) {
    if (!index)
        return;
    for (size_t i = 0; i < index->count; ++i) {
        TrigramEntry *entry = &index->entries[i];
        FileListNode *node = entry->file_list;
        while (node) {
            FileListNode *next = node->next;
            free(node->filename);
            free(node);
            node = next;
        }
    }
    free(index->entries);
    free(index);
}
static int grow_trigram_entries(TrigramIndex *index) {
    size_t new_capacity = (index->capacity == 0) ? 16 : index->capacity * 2;
    TrigramEntry *new_entries = realloc(index->entries, new_capacity * sizeof(TrigramEntry));
    if (!new_entries) {
        return -1;
    }
    index->entries = new_entries;
    index->capacity = new_capacity;
    return 0;
}
int add_file_to_index(TrigramIndex *index, const char *filename) {
    if (!index || !filename)
        return -1;
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("fopen");
        return -1;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("fseek");
        fclose(file);
        return -1;
    }
    long file_size = ftell(file);
    if (file_size < 0) {
        perror("ftell");
        fclose(file);
        return -1;
    }
    rewind(file);
    char *content = (char *)malloc(file_size + 1);
    if (!content) {
        perror("malloc");
        fclose(file);
        return -1;
    }
    size_t read_size = fread(content, 1, file_size, file);
    if ((long)read_size != file_size) {
        perror("fread");
        free(content);
        fclose(file);
        return -1;
    }
    content[file_size] = '\0';
    fclose(file);
    if (file_size < 3) {
        free(content);
        return 0;
    }
    for (size_t i = 0; i <= file_size - 3; ++i) {
        uint32_t trigram = encode_trigram(&content[i]);
        size_t j;
        for (j = 0; j < index->count; ++j) {
            if (index->entries[j].trigram == trigram)
                break;
        }
        if (j == index->count) {
            if (index->count == index->capacity) {
                if (grow_trigram_entries(index) != 0) {
                    free(content);
                    return -1;
                }
            }
            index->entries[j].trigram = trigram;
            index->entries[j].file_list = NULL;
            index->count++;
        }
        TrigramEntry *entry = &index->entries[j];
        FileListNode *node = entry->file_list;
        int found = 0;
        while (node) {
            if (strcmp(node->filename, filename) == 0) {
                found = 1;
                break;
            }
            node = node->next;
        }
        if (!found) {
            FileListNode *new_node = (FileListNode *)malloc(sizeof(FileListNode));
            if (!new_node) {
                free(content);
                return -1;
            }
            new_node->filename = strdup(filename);
            if (!new_node->filename) {
                free(new_node);
                free(content);
                return -1;
            }
            new_node->next = entry->file_list;
            entry->file_list = new_node;
        }
    }
    free(content);
    return 0;
}
int search_trigram_index(TrigramIndex *index, const char *query, char ***results, size_t *results_count) {
    if (!index || !query || !results || !results_count)
        return -1;
    size_t query_length = strlen(query);
    if (query_length < 3) {
        *results = NULL;
        *results_count = 0;
        return 0;
    }
    size_t num_trigrams = query_length - 2;
    uint32_t *query_trigrams = (uint32_t *)malloc(num_trigrams * sizeof(uint32_t));
    if (!query_trigrams)
        return -1;
    for (size_t i = 0; i < num_trigrams; ++i) {
        query_trigrams[i] = encode_trigram(&query[i]);
    }
    FileListNode **file_lists = (FileListNode **)malloc(num_trigrams * sizeof(FileListNode *));
    if (!file_lists) {
        free(query_trigrams);
        return -1;
    }
    for (size_t i = 0; i < num_trigrams; ++i) {
        uint32_t trigram = query_trigrams[i];
        TrigramEntry *entry = NULL;
        for (size_t j = 0; j < index->count; ++j) {
            if (index->entries[j].trigram == trigram) {
                entry = &index->entries[j];
                break;
            }
        }
        if (!entry) {
            free(query_trigrams);
            free(file_lists);
            *results = NULL;
            *results_count = 0;
            return 0;
        }
        file_lists[i] = entry->file_list;
    }
    typedef struct {
        char *filename;
        size_t count;
    } FileCount;
    size_t file_counts_capacity = 16;
    size_t file_counts_count = 0;
    FileCount *file_counts = (FileCount *)malloc(file_counts_capacity * sizeof(FileCount));
    if (!file_counts) {
        free(query_trigrams);
        free(file_lists);
        return -1;
    }
    for (size_t i = 0; i < num_trigrams; ++i) {
        FileListNode *node = file_lists[i];
        while (node) {
            size_t j;
            for (j = 0; j < file_counts_count; ++j) {
                if (strcmp(file_counts[j].filename, node->filename) == 0) {
                    file_counts[j].count++;
                    break;
                }
            }
            if (j == file_counts_count) {
                if (file_counts_count == file_counts_capacity) {
                    size_t new_capacity = file_counts_capacity * 2;
                    FileCount *new_counts = realloc(file_counts, new_capacity * sizeof(FileCount));
                    if (!new_counts) {
                        free(query_trigrams);
                        free(file_lists);
                        for (size_t k = 0; k < file_counts_count; ++k)
                            free(file_counts[k].filename);
                        free(file_counts);
                        return -1;
                    }
                    file_counts = new_counts;
                    file_counts_capacity = new_capacity;
                }
                file_counts[file_counts_count].filename = strdup(node->filename);
                if (!file_counts[file_counts_count].filename) {
                    free(query_trigrams);
                    free(file_lists);
                    for (size_t k = 0; k < file_counts_count; ++k)
                        free(file_counts[k].filename);
                    free(file_counts);
                    return -1;
                }
                file_counts[file_counts_count].count = 1;
                file_counts_count++;
            }
            node = node->next;
        }
    }
    char **candidate_files = NULL;
    size_t candidate_count = 0;
    size_t candidate_capacity = 16;
    candidate_files = (char **)malloc(candidate_capacity * sizeof(char *));
    if (!candidate_files) {
        free(query_trigrams);
        free(file_lists);
        for (size_t k = 0; k < file_counts_count; ++k)
            free(file_counts[k].filename);
        free(file_counts);
        return -1;
    }
    for (size_t i = 0; i < file_counts_count; ++i) {
        if (file_counts[i].count == num_trigrams) {
            if (candidate_count == candidate_capacity) {
                size_t new_capacity = candidate_capacity * 2;
                char **new_candidates = realloc(candidate_files, new_capacity * sizeof(char *));
                if (!new_candidates) {
                    free(query_trigrams);
                    free(file_lists);
                    for (size_t k = 0; k < file_counts_count; ++k)
                        free(file_counts[k].filename);
                    free(file_counts);
                    for (size_t k = 0; k < candidate_count; ++k)
                        free(candidate_files[k]);
                    free(candidate_files);
                    return -1;
                }
                candidate_files = new_candidates;
                candidate_capacity = new_capacity;
            }
            candidate_files[candidate_count++] = strdup(file_counts[i].filename);
            if (!candidate_files[candidate_count - 1]) {
                free(query_trigrams);
                free(file_lists);
                for (size_t k = 0; k < file_counts_count; ++k)
                    free(file_counts[k].filename);
                free(file_counts);
                for (size_t k = 0; k < candidate_count - 1; ++k)
                    free(candidate_files[k]);
                free(candidate_files);
                return -1;
            }
        }
        free(file_counts[i].filename);
    }
    free(file_counts);
    size_t result_capacity = candidate_count;
    size_t result_count = 0;
    char **result_files = (char **)malloc(result_capacity * sizeof(char *));
    if (!result_files) {
        free(query_trigrams);
        free(file_lists);
        for (size_t k = 0; k < candidate_count; ++k)
            free(candidate_files[k]);
        free(candidate_files);
        return -1;
    }
    for (size_t i = 0; i < candidate_count; ++i) {
        const char *filename = candidate_files[i];
        FILE *file = fopen(filename, "rb");
        if (!file) {
            perror("fopen");
            continue;
        }
        if (fseek(file, 0, SEEK_END) != 0) {
            fclose(file);
            perror("fseek");
            continue;
        }
        long file_size = ftell(file);
        if (file_size < 0) {
            fclose(file);
            perror("ftell");
            continue;
        }
        rewind(file);
        char *content = (char *)malloc(file_size + 1);
        if (!content) {
            fclose(file);
            perror("malloc");
            continue;
        }
        size_t read_size = fread(content, 1, file_size, file);
        if ((long)read_size != file_size) {
            fclose(file);
            free(content);
            perror("fread");
            continue;
        }
        content[file_size] = '\0';
        fclose(file);
        if (strstr(content, query) != NULL) {
            result_files[result_count++] = strdup(filename);
            if (!result_files[result_count - 1]) {
                free(content);
                free(query_trigrams);
                free(file_lists);
                for (size_t k = 0; k < candidate_count; ++k)
                    free(candidate_files[k]);
                free(candidate_files);
                for (size_t k = 0; k < result_count - 1; ++k)
                    free(result_files[k]);
                free(result_files);
                return -1;
            }
        }
        free(content);
    }
    free(query_trigrams);
    free(file_lists);
    for (size_t k = 0; k < candidate_count; ++k)
        free(candidate_files[k]);
    free(candidate_files);
    *results = result_files;
    *results_count = result_count;
    return 0;
}
