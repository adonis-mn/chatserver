#include"UserModel.hpp"
#include"db.hpp"
#include<iostream>
//将user类插入到数据库当中
bool UserModel::insert(User& user)
{
    char sql[1024] = {0};
    //用于字符串的连接
    sprintf(sql,"insert into user(name ,password,state) values('%s','%s','%s')" , user.getName().c_str(),user.getPwd().c_str(),user.getState().c_str());
     //刚注册还没登录当然是offline
    MySQL mysql;
    if(mysql.connect())
    {
        //连接上了后，查询user并更新user,将sql传进去。
        if(mysql.update(sql))       //update进行一个数据的插入
        {
            //获取插入成功的用户数据,生成id
            user.setid(mysql.getid());
            return true;
        }

    }
    return false;
}

User UserModel::query(int id)
{
    //根据用户id,查询
    //组装sql语句
    char sql[1024]={0};
    sprintf(sql,"select * from user where  id = %d",id);
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if( res != nullptr)
        {
            //查询到了，要从user表中获取数据（name,password）
            MYSQL_ROW row = mysql_fetch_row(res) ; //通过 mysql_fetch_row 函数来获取user表中的数据
            if(row !=nullptr)
            {
                User user;
                user.setid(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                //将动态分配的MYSQL_RES 资源释放掉
                mysql_free_result(res);
                return user;
            }
        }
    }
    //通过返回User(),构造函数id初始化为-1，来提示出错了
    return User(); 
}
    //更新用户的状态信息
bool  UserModel::updatestate(User &user)
{
    char sql[1024] = {0};
    sprintf(sql,"update  user set state = '%s' where id = %d" , user.getState().c_str(),user.getid());
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

void UserModel::resetState()
{
    char sql[1024] = "update user set state ='offline' where state ='online'" ;
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}
