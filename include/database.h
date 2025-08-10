#ifndef DATABASE_H
#define DATABASE_H

#include <cjson/cJSON.h>

int init_storage();
static int create_parent_dirs(const char *path);
int save_file_and_metadata(const char *filename, int version, const char *contents, const char *hash, const char *action);
int delete_file_and_metadata(const char *filename, int version, const char *contents, const char *hash, const char *action);
int edit_file_and_metadata(const char *filename, int version, const char *contents, const char *hash, const char *action);
cJSON *load_file_and_metadata(const char *filename);
cJSON *get_missing_files(cJSON *summary);

#endif
