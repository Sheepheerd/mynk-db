#include <sqlite3.h>

int main(void) {
    sqlite3 *db;
    return sqlite3_open(":memory:", &db);
}
