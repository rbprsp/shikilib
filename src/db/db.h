#ifndef __DB_H__
#define __DB_H__

#include <sqlite3.h>
#include <string>

class Database
{
private:
    const char* DB_NAME = "shikilib.db";
    sqlite3* db;
    sqlite3_stmt* stmt;

public:
    void MergeBase();
    void AddShikiIdByName(std::string title);
};                          

#endif