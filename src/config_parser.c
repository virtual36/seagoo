#include "seagoo.h"
#include <libconfig.h>

/* Load configuration from a file */
int load_config(const char *filename, config_t *cfg) {
    config_init(cfg);

    if (!config_read_file(cfg, filename)) {
        fprintf(stderr, "error reading the config file: %s\n", config_error_text(cfg));
        config_destroy(cfg);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* Store configuration to a file */
int store_config(const char *filename, const config_t *cfg) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("error opening config file for writing");
        return EXIT_FAILURE;
    }

    config_write(cfg, file);
    fclose(file);
    return EXIT_SUCCESS;
}

/* Write a default template configuration file */
int write_default_config(const char *filename) {
    config_t cfg;
    config_init(&cfg);

    config_setting_t *root = config_root_setting(&cfg);
    
    // Add default settings to the config_t object
    config_setting_t *setting = config_setting_add(root, "grep_binary", CONFIG_TYPE_STRING);
    config_setting_set_string(setting, "/usr/bin/rg"); /* just example */

    setting = config_setting_add(root, "sqlite_db", CONFIG_TYPE_STRING);
    config_setting_set_string(setting, "~/.seagoo-index.db"); /* just example */

    setting = config_setting_add(root, "num_results_to_list", CONFIG_TYPE_INT);
    config_setting_set_int(setting, 5);

    setting = config_setting_add(root, "graph_pruning_distance", CONFIG_TYPE_INT);
    config_setting_set_int(setting, 15);

    setting = config_setting_add(root, "parse_comments", CONFIG_TYPE_BOOL);
    config_setting_set_bool(setting, 0);

    if (store_config(filename, &cfg) != EXIT_SUCCESS) {
        config_destroy(&cfg);
        return EXIT_FAILURE;
    }

    config_destroy(&cfg);
    return EXIT_SUCCESS;
}
