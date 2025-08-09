#include "database.h"
#include "unistd.h"
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#define SYNC_FILES_DIR "./"
#define SYNC_METADATA_DIR "./"

int init_storage() {
    // struct stat st = {0};
    // if (stat(SYNC_FILES_DIR, &st) == -1) {
    //     if (mkdir(SYNC_FILES_DIR, 0700) != 0) {
    //         fprintf(stderr, "Failed to create %s\n", SYNC_FILES_DIR);
    //         return 0;
    //     }
    // }
    // if (stat(SYNC_METADATA_DIR, &st) == -1) {
    //     if (mkdir(SYNC_METADATA_DIR, 0700) != 0) {
    //         fprintf(stderr, "Failed to create %s\n", SYNC_METADATA_DIR);
    //         return 0;
    //     }
    // }
    // return 1;
}

static int create_parent_dirs(const char *path) {
    char *tmp = strdup(path);
    if (!tmp) {
        fprintf(stderr, "Failed to allocate memory for path\n");
        return 0;
    }

    char *p = tmp;
    if (strncmp(p, SYNC_FILES_DIR, strlen(SYNC_FILES_DIR)) == 0) {
        p += strlen(SYNC_FILES_DIR); // Skip SYNC_FILES_DIR prefix
    }

    for (char *slash = strchr(p, '/'); slash; slash = strchr(slash + 1, '/')) {
        *slash = '\0';
        char *dir_path = malloc(strlen(SYNC_FILES_DIR) + strlen(tmp) + 1);
        if (!dir_path) {
            fprintf(stderr, "Failed to allocate memory for dir_path\n");
            free(tmp);
            return 0;
        }
        sprintf(dir_path, "%s%s", SYNC_FILES_DIR, tmp);

        struct stat st = {0};
        if (stat(dir_path, &st) == -1) {
            if (mkdir(dir_path, 0700) != 0 && errno != EEXIST) {
                fprintf(stderr, "Failed to create directory %s: %s\n", dir_path, strerror(errno));
                free(dir_path);
                free(tmp);
                return 0;
            }
        }
        free(dir_path);
        *slash = '/';
    }

    free(tmp);
    return 1;
}

int save_file_and_metadata(const char *filename, int version, const char *contents, const char *hash, const char *action) {
    // Ensure parent directories exist
    if (!create_parent_dirs(filename)) {
        fprintf(stderr, "Failed to create parent directories for %s\n", filename);
        return 0;
    }

    // Save file to disk
    char *path = malloc(strlen(SYNC_FILES_DIR) + strlen(filename) + 1);
    if (!path) {
        fprintf(stderr, "Failed to allocate path\n");
        return 0;
    }

    sprintf(path, "%s%s", SYNC_FILES_DIR, filename);

    if (access(path, F_OK) == 0) {
        fprintf(stderr, "File Already Exists\n", path, strerror(errno));
        free(path);
        return 0;
    }

    FILE *file = fopen(path, "w");
    if (!file) {
        fprintf(stderr, "Failed to open file %s: %s\n", path, strerror(errno));
        free(path);
        return 0;
    }

    if (fwrite(contents, 1, strlen(contents), file) != strlen(contents)) {
        fprintf(stderr, "Failed to write to file %s\n", path);
        fclose(file);
        free(path);
        return 0;
    }

    fclose(file);

    cJSON *metadata = cJSON_CreateObject();
    if (!metadata) {
        fprintf(stderr, "Failed to create metadata object\n");
        free(path);
        return 0;
    }

    cJSON_AddStringToObject(metadata, "filename", filename);
    cJSON_AddNumberToObject(metadata, "version", version);
    cJSON_AddStringToObject(metadata, "hash", hash);
    cJSON_AddStringToObject(metadata, "path", path);

    time_t now = time(NULL);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    cJSON_AddStringToObject(metadata, "last_modified", timestamp);

    // Ensure metadata directory exists
    struct stat st = {0};
    if (stat(SYNC_METADATA_DIR, &st) == -1) {
        if (mkdir(SYNC_METADATA_DIR, 0700) != 0 && errno != EEXIST) {
            fprintf(stderr, "Failed to create %s: %s\n", SYNC_METADATA_DIR, strerror(errno));
            cJSON_Delete(metadata);
            free(path);
            return 0;
        }
    }

    char *meta_path = malloc(strlen(SYNC_METADATA_DIR) + strlen(filename) + 6);
    if (!meta_path) {
        fprintf(stderr, "Failed to allocate metadata path\n");
        cJSON_Delete(metadata);
        free(path);
        return 0;
    }
    sprintf(meta_path, "%s%s.json", SYNC_METADATA_DIR, filename);

    FILE *meta_file = fopen(meta_path, "w");
    if (!meta_file) {
        fprintf(stderr, "Failed to open metadata file %s: %s\n", meta_path, strerror(errno));
        cJSON_Delete(metadata);
        free(path);
        free(meta_path);
        return 0;
    }

    char *meta_str = cJSON_Print(metadata);
    if (!meta_str || fwrite(meta_str, 1, strlen(meta_str), meta_file) != strlen(meta_str)) {
        fprintf(stderr, "Failed to write metadata to %s\n", meta_path);
        cJSON_Delete(metadata);
        free(meta_str);
        free(path);
        free(meta_path);
        fclose(meta_file);
        return 0;
    }

    fclose(meta_file);
    cJSON_Delete(metadata);
    free(meta_str);
    free(meta_path);

    free(path);
    return 1;
}


int delete_file_and_metadata(const char *filename, int version, const char *contents, const char *hash, const char *action) {
}

cJSON *load_file_and_metadata(const char *filename) {

}
cJSON *get_missing_files(cJSON *summary) {

}
