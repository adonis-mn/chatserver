#ifndef DB_H
#define DB_H


#include<string>
#include<mysql/mysql.h>
using namespace std;
//封装mysqld的内部操作，主要是针对对mysql内部操作
class MySQL
{
public:
  
    MySQL();
    ~MySQL();
    bool connect();
    bool update(string sql);
    int getid();
    MYSQL_RES *query(string sql);
private:
    MYSQL* _conn;
};

#endif