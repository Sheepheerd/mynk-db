#include <stddef.h>
#include "parser.h"
#include "database.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <string.h>

/**
  * Check the summary first, if there are changes that havn't been synced then send back the changes first
*/

char *parse_post_sync(const char *data) {
    cJSON *json = cJSON_Parse(data);
    if (!json) {
        fprintf(stderr, "JSON parse error\n");
        return strdup("[]");
    }

    if (!cJSON_IsObject(json)) {
        cJSON_Delete(json);
        fprintf(stderr, "JSON is not an object\n");
        return strdup("[]");
    }

    cJSON *files = cJSON_GetObjectItemCaseSensitive(json, "files");
    if (files && cJSON_IsArray(files)) {
        cJSON *file;
        cJSON_ArrayForEach(file, files) {
            if (!cJSON_IsObject(file)) {
                fprintf(stderr, "Error: File item is not an object\n");
                continue;
            }
            cJSON *filename = cJSON_GetObjectItemCaseSensitive(file, "filename");
            cJSON *version = cJSON_GetObjectItemCaseSensitive(file, "version");
            cJSON *contents = cJSON_GetObjectItemCaseSensitive(file, "contents");
            cJSON *hash = cJSON_GetObjectItemCaseSensitive(file, "hash");
            cJSON *action = cJSON_GetObjectItemCaseSensitive(file, "action");

            if (!cJSON_IsString(filename) || !filename->valuestring || filename->valuestring[0] == '\0' ||
                    !cJSON_IsNumber(version) ||
                    !cJSON_IsString(contents) || !contents->valuestring ||
                    !cJSON_IsString(hash) || !hash->valuestring || hash->valuestring[0] == '\0' ||
                    !cJSON_IsString(action) || !action->valuestring || action->valuestring[0] == '\0')
            {
                fprintf(stderr, "Error: Invalid or missing fields in file item\n");
                continue;
            }

            printf("File: %s, Version: %d, Contents: %s, Hash: %s, Action: %s\n",
                   filename->valuestring, version->valueint, contents->valuestring,
                   hash->valuestring, action->valuestring);

            if (strcmp(action->valuestring, "create") == 0) {
                if (!save_file_and_metadata(filename->valuestring, version->valueint, contents->valuestring, hash->valuestring, "write")) {
                    fprintf(stderr, "Failed to create file %s\n", filename->valuestring);
                }
            } else if (strcmp(action->valuestring, "edit") == 0) {
                if (!edit_file_and_metadata(filename->valuestring, version->valueint, contents->valuestring, hash->valuestring, action->valuestring)) {
                    fprintf(stderr, "Failed to edit file %s\n", filename->valuestring);
                }
            } else if (strcmp(action->valuestring, "delete") == 0) {
                if (!delete_file_and_metadata(filename->valuestring, version->valueint, contents->valuestring, hash->valuestring, action->valuestring)) {
                    fprintf(stderr, "Failed to delete file %s\n", filename->valuestring);
                }
            } else {
                fprintf(stderr, "Invalid action: %s for file %s\n", action->valuestring, filename->valuestring);
            }
        }
    }

    cJSON *summary = cJSON_GetObjectItemCaseSensitive(json, "summary");
    if (!cJSON_IsArray(summary)) {
        cJSON_Delete(json);
        return strdup("[]");
    }

    cJSON *missing_files = get_missing_files(summary);
    if (!missing_files) {
        cJSON_Delete(json);
        return strdup("[]");
    }

    char *response_str = cJSON_Print(missing_files);
    cJSON_Delete(missing_files);
    cJSON_Delete(json);

    return response_str ? response_str : strdup("[]");
}
