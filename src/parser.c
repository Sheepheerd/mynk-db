#include <stddef.h>
#include <collectc/cc_hashtable.h>
#include "parser.h"
#include <cjson/cJSON.h>
#include <stdio.h>



int sync_parse(char *data) {
    cJSON *json = cJSON_Parse(data);
    if (json == NULL)
    {
        return 0;
    }

    // Ensure JSON is an object
    if (!cJSON_IsObject(json))
    {
        cJSON_Delete(json);
        return 0;
    }

    // Process required summary array
    cJSON *summary = cJSON_GetObjectItemCaseSensitive(json, "summary");
    if (!cJSON_IsArray(summary))
    {
        cJSON_Delete(json);
        return 0;
    }

    cJSON *summary_item;
    cJSON_ArrayForEach(summary_item, summary)
    {
        if (!cJSON_IsObject(summary_item))
        {
            fprintf(stderr, "Error: Summary item is not an object\n");
            continue;
        }
        cJSON *filename = cJSON_GetObjectItemCaseSensitive(summary_item, "filename");
        cJSON *contents_hash = cJSON_GetObjectItemCaseSensitive(summary_item, "contents_hash");
        if (cJSON_IsString(filename) && filename->valuestring && filename->valuestring[0] != '\0' &&
                cJSON_IsString(contents_hash) && contents_hash->valuestring && contents_hash->valuestring[0] != '\0')
        {
            printf("Summary - Filename: %s, Contents Hash: %s\n",
                   filename->valuestring, contents_hash->valuestring);
        }
        else
        {
            fprintf(stderr, "Error: Invalid or missing fields in summary item\n");
        }
    }

    // Process optional files array if present
    cJSON *files = cJSON_GetObjectItemCaseSensitive(json, "files");
    if (files && cJSON_IsArray(files))
    {
        cJSON *file;
        cJSON_ArrayForEach(file, files)
        {
            if (!cJSON_IsObject(file))
            {
                fprintf(stderr, "Error: File item is not an object\n");
                continue;
            }
            cJSON *filename = cJSON_GetObjectItemCaseSensitive(file, "filename");
            cJSON *version = cJSON_GetObjectItemCaseSensitive(file, "version");
            cJSON *contents = cJSON_GetObjectItemCaseSensitive(file, "contents");
            cJSON *hash = cJSON_GetObjectItemCaseSensitive(file, "hash");
            if (cJSON_IsString(filename) && filename->valuestring && filename->valuestring[0] != '\0' &&
                    cJSON_IsNumber(version) &&
                    cJSON_IsString(contents) && contents->valuestring &&
                    cJSON_IsString(hash) && hash->valuestring && hash->valuestring[0] != '\0')
            {
                printf("File: %s, Version: %d, Contents: %s, Hash: %s\n",
                       filename->valuestring, version->valueint, contents->valuestring, hash->valuestring);
                // Process file data as needed (e.g., save contents, verify hash)
            }
            else
            {
                fprintf(stderr, "Error: Invalid or missing fields in file item\n");
            }
        }
    }

    // Clean up
    cJSON_Delete(json);
    return 1;

}
