#include <sqlite3.h>
#include <stdio.h>
int main(void) {
    sqlite3 *db;
    printf("Hello, world!\n");
    return sqlite3_open(":memory:", &db);
}
