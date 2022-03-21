#include"db.hpp"

#include<muduo/base/Logging.h>
//配置数据库连接
static string server = "127.0.0.1" ;
static string user ="root" ;
static string password = "123456" ;
static string dbname = "chat" ;

//构造函数用于初始化数据库连接
MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}

MySQL:: ~MySQL()
{
    if(_conn != nullptr)
    {
        mysql_close(_conn);
    }
}
int MySQL:: getid()
{
    return mysql_insert_id(_conn);      //通过这个函数来得到插入到表中数据的id
}
//连接数据库
bool MySQL::connect()
{
    MYSQL *p =mysql_real_connect(_conn,server.c_str(),user.c_str(),password.c_str(),dbname.c_str(),3306,nullptr,0);
    if(p!=nullptr){
        //c/c++代码是ASCII码，从mysql上面拉下来不是中文显示；
        mysql_query(_conn , "set names gbk");
        LOG_INFO<<"connect mysql success!" ;
    }
    else 
    {
        LOG_INFO<<"connect mysql failed!";    //为了方便出错找到问题
    }
    return p;   //这里不是空指正就是true
}
bool MySQL::update(string sql)
{
    if(mysql_query(_conn,sql.c_str()))
    {
        LOG_INFO<<__FILE__ <<":" <<__LINE__ <<":" <<sql <<"更新失败!";
        return false;   //没查询到
    }
    return true;
}
//查询操作
MYSQL_RES*  MySQL::query(string sql)
{
    if(mysql_query(_conn,sql.c_str()))
    {
        LOG_INFO<<__FILE__<<":" <<__LINE__<<": "<<sql <<"查询失败";
        return nullptr;
    }
    return mysql_use_result(_conn);
}