#include"friendmodel.hpp"

vector<User> FriendModel::query(int userid )
{
    char sql[1024] = {0};
    //从user表中userid 中筛选出 friend表中friendid
    sprintf(sql,"select a.id,a.name,a.state from user a inner join friend b on b.friendid = a.id where b.userid=%d", userid )  ;
    MySQL mysql;
    vector<User>vec;
    if(mysql.connect())
    {
        MYSQL_RES *res;
        res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row ;
            while((row = mysql_fetch_row(res))!= nullptr)
            {
                User user;
                user.setid(atoi(row[0]));
                user.setName((row[1]));
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
       
    }
    return vec;
}

void FriendModel::insert(int userid, int friendid)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values(%d, %s)", userid, friendid );

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}