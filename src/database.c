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
    // nothing to see here
}

static char *create_conflict_filename(const char *filename, const char *device_id) {
    time_t now = time(NULL);
    char timestamp[16];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H%M%S", gmtime(&now));

    char *ext = strrchr(filename, '.');
    size_t base_len = ext ? (ext - filename) : strlen(filename);
    char *conflict_filename = malloc(base_len + strlen(".sync-conflict-") + strlen(timestamp) + strlen("-") + strlen(device_id) + (ext ? strlen(ext) : 0) + 1);
    if (!conflict_filename) {
        fprintf(stderr, "Failed to allocate conflict filename\n");
        return NULL;
    }

    snprintf(conflict_filename, base_len + 1, "%s", filename);
    sprintf(conflict_filename + base_len, ".sync-conflict-%s-%s%s", timestamp, device_id, ext ? ext : "");
    return conflict_filename;
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
    if (!create_parent_dirs(filename)) {
        fprintf(stderr, "Failed to create parent directories for %s\n", filename);
        return 0;
    }

    char *path = malloc(strlen(SYNC_FILES_DIR) + strlen(filename) + 1);
    if (!path) {
        fprintf(stderr, "Failed to allocate path\n");
        return 0;
    }
    sprintf(path, "%s%s", SYNC_FILES_DIR, filename);

    if (strcmp(action, "edit") != 0 && strcmp(action, "conflict_only") != 0 && access(path, F_OK) == 0) {
        fprintf(stderr, "File already exists: %s\n", path);
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

    if (strcmp(action, "conflict_only") != 0) {
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
    }

    free(path);
    return 1;
}

int edit_file_and_metadata(const char *filename, int version, const char *contents, const char *hash, const char *action) {
    cJSON *existing_metadata = load_file_and_metadata(filename);
    int existing_version = -1;

    if (existing_metadata) {
        cJSON *version_item = cJSON_GetObjectItemCaseSensitive(existing_metadata, "version");
        if (cJSON_IsNumber(version_item)) {
            existing_version = version_item->valueint;
        }
        cJSON_Delete(existing_metadata);
    }

    if (existing_version == version) {
        char device_id[16];
        snprintf(device_id, sizeof(device_id), "DEVICE_%d", (int)time(NULL) % 10000);
        char *conflict_filename = create_conflict_filename(filename, device_id);
        if (!conflict_filename) {
            return 0;
        }

        int result = save_file_and_metadata(conflict_filename, version, contents, hash, "conflict_only");
        free(conflict_filename);
        return result;
    }

    if (version > existing_version) {
        return save_file_and_metadata(filename, version, contents, hash, action);
    }

    fprintf(stderr, "Version %d is not greater than existing version %d for %s\n", version, existing_version, filename);
    return 0;
}

int delete_file_and_metadata(const char *filename, int version, const char *contents, const char *hash, const char *action) {
    (void)contents;
    (void)hash;
    (void)action;

    cJSON *existing_metadata = load_file_and_metadata(filename);
    int existing_version = -1;

    if (existing_metadata) {
        cJSON *version_item = cJSON_GetObjectItemCaseSensitive(existing_metadata, "version");
        if (cJSON_IsNumber(version_item)) {
            existing_version = version_item->valueint;
        }
        cJSON_Delete(existing_metadata);
    } else {
        fprintf(stderr, "No metadata found for %s, cannot delete\n", filename);
        return 0;
    }

    if (version != existing_version) {
        fprintf(stderr, "Version mismatch: provided %d, existing %d for %s\n", version, existing_version, filename);
        return 0;
    }

    char *path = malloc(strlen(SYNC_FILES_DIR) + strlen(filename) + 1);
    if (!path) {
        fprintf(stderr, "Failed to allocate path\n");
        return 0;
    }
    sprintf(path, "%s%s", SYNC_FILES_DIR, filename);

    char *meta_path = malloc(strlen(SYNC_METADATA_DIR) + strlen(filename) + 6);
    if (!meta_path) {
        fprintf(stderr, "Failed to allocate metadata path\n");
        free(path);
        return 0;
    }
    sprintf(meta_path, "%s%s.json", SYNC_METADATA_DIR, filename);

    if (unlink(path) != 0) {
        fprintf(stderr, "Failed to delete file %s: %s\n", path, strerror(errno));
        free(path);
        free(meta_path);
        return 0;
    }

    if (unlink(meta_path) != 0) {
        fprintf(stderr, "Failed to delete metadata file %s: %s\n", meta_path, strerror(errno));
        free(path);
        free(meta_path);
        return 0;
    }

    free(path);
    free(meta_path);
    return 1;
}


cJSON *load_file_and_metadata(const char *filename) {
    char *meta_path = malloc(strlen(SYNC_METADATA_DIR) + strlen(filename) + 6);
    if (!meta_path) {
        fprintf(stderr, "Failed to allocate metadata path\n");
        return NULL;
    }
    sprintf(meta_path, "%s%s.json", SYNC_METADATA_DIR, filename);

    FILE *meta_file = fopen(meta_path, "r");
    if (!meta_file) {
        free(meta_path);
        return NULL;
    }

    fseek(meta_file, 0, SEEK_END);
    long size = ftell(meta_file);
    fseek(meta_file, 0, SEEK_SET);

    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(meta_file);
        free(meta_path);
        return NULL;
    }

    if (fread(buffer, 1, size, meta_file) != (size_t)size) {
        fprintf(stderr, "Failed to read metadata from %s\n", meta_path);
        free(buffer);
        fclose(meta_file);
        free(meta_path);
        return NULL;
    }
    buffer[size] = '\0';

    fclose(meta_file);
    free(meta_path);

    cJSON *metadata = cJSON_Parse(buffer);
    free(buffer);
    return metadata;
}

cJSON *get_missing_files(cJSON *summary) {
    cJSON *response_array = cJSON_CreateArray();
    if (!response_array) {
        fprintf(stderr, "Failed to create response array\n");
        return NULL;
    }

    DIR *dir = opendir(SYNC_METADATA_DIR);
    if (!dir) {
        fprintf(stderr, "Failed to open metadata directory %s: %s\n", SYNC_METADATA_DIR, strerror(errno));
        cJSON_Delete(response_array);
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (strstr(entry->d_name, ".json")) {
            char *local_filename = strdup(entry->d_name);
            local_filename[strlen(local_filename) - 5] = '\0'; // Remove .json

            cJSON *local_metadata = load_file_and_metadata(local_filename);
            if (local_metadata) {
                cJSON *local_hash_item = cJSON_GetObjectItemCaseSensitive(local_metadata, "hash");
                cJSON *local_version_item = cJSON_GetObjectItemCaseSensitive(local_metadata, "version");
                cJSON *local_last_modified_item = cJSON_GetObjectItemCaseSensitive(local_metadata, "last_modified");
                cJSON *local_path_item = cJSON_GetObjectItemCaseSensitive(local_metadata, "path");

                if (cJSON_IsString(local_hash_item) && cJSON_IsNumber(local_version_item) &&
                        cJSON_IsString(local_last_modified_item) && cJSON_IsString(local_path_item)) {
                    // Check if file exists in summary
                    int found_in_summary = 0;
                    cJSON *summary_item;
                    cJSON_ArrayForEach(summary_item, summary) {
                        if (!cJSON_IsObject(summary_item)) {
                            continue;
                        }
                        cJSON *sum_filename = cJSON_GetObjectItemCaseSensitive(summary_item, "filename");
                        cJSON *sum_hash = cJSON_GetObjectItemCaseSensitive(summary_item, "contents_hash");
                        if (cJSON_IsString(sum_filename) && cJSON_IsString(sum_hash) &&
                                strcmp(sum_filename->valuestring, local_filename) == 0) {
                            found_in_summary = 1;
                            if (strcmp(sum_hash->valuestring, local_hash_item->valuestring) != 0) {
                                cJSON *file_obj = cJSON_CreateObject();
                                cJSON_AddStringToObject(file_obj, "filename", local_filename);
                                cJSON_AddNumberToObject(file_obj, "version", local_version_item->valueint);
                                cJSON_AddStringToObject(file_obj, "hash", local_hash_item->valuestring);
                                cJSON_AddStringToObject(file_obj, "last_modified", local_last_modified_item->valuestring);

                                FILE *file = fopen(local_path_item->valuestring, "r");
                                if (file) {
                                    fseek(file, 0, SEEK_END);
                                    long size = ftell(file);
                                    fseek(file, 0, SEEK_SET);
                                    char *local_contents = malloc(size + 1);
                                    if (local_contents) {
                                        fread(local_contents, 1, size, file);
                                        local_contents[size] = '\0';
                                        cJSON_AddStringToObject(file_obj, "contents", local_contents);
                                        free(local_contents);
                                    }
                                    fclose(file);
                                }

                                cJSON_AddItemToArray(response_array, file_obj);
                            }
                            break;
                        }
                    }

                    if (!found_in_summary) {
                        cJSON *file_obj = cJSON_CreateObject();
                        cJSON_AddStringToObject(file_obj, "filename", local_filename);
                        cJSON_AddNumberToObject(file_obj, "version", local_version_item->valueint);
                        cJSON_AddStringToObject(file_obj, "hash", local_hash_item->valuestring);
                        cJSON_AddStringToObject(file_obj, "last_modified", local_last_modified_item->valuestring);

                        FILE *file = fopen(local_path_item->valuestring, "r");
                        if (file) {
                            fseek(file, 0, SEEK_END);
                            long size = ftell(file);
                            fseek(file, 0, SEEK_SET);
                            char *local_contents = malloc(size + 1);
                            if (local_contents) {
                                fread(local_contents, 1, size, file);
                                local_contents[size] = '\0';
                                cJSON_AddStringToObject(file_obj, "contents", local_contents);
                                free(local_contents);
                            }
                            fclose(file);
                        }

                        cJSON_AddItemToArray(response_array, file_obj);
                    }
                }
                cJSON_Delete(local_metadata);
            }
            free(local_filename);
        }
    }
    closedir(dir);

    return response_array;
}
