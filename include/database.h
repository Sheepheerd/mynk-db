#ifndef DATABASE_H
#define DATABASE_H

#include <cjson/cJSON.h>

int init_storage();
int save_file_metadata(const char *filename, int version, const char *contents, const char *hash, const char *action);
cJSON *load_file_metadata(const char *filename);
cJSON *get_missing_files(cJSON *summary);

#endif
