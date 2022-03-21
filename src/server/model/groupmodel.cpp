#include"db.hpp"
#include<vector>
#include"groupuser.hpp"
#include"groupmodel.hpp"
using namespace std;

 bool GroupModel::createGroup(Group& group)
{
    char sql[1024] = {0};
    sprintf(sql,"insert int allgroup(groupname ,groupdesc) value('%s', '%s')",group.getname().c_str(),group.getdesc().c_str());
    MySQL mysql;
    if(mysql.connect()) 
    {
        if(mysql.update(sql))
        {
            group.setid(mysql.getid());
            return true;
        }
    }
    return false;
}

void GroupModel::addGroup(int userid,int groupid,string role)
{
    char sql[1024]={0};
    sprintf(sql,"insert into groupuser values(%d,%d,'%s')",groupid,userid,role.c_str());
    MySQL mysql;
    if(mysql.connect())
    {
    mysql.update(sql);
    }   
    
    
} 

vector<Group>  GroupModel::queryGroups(int userid)
{
    //先根据userid在groupuser表中查询出所属的群组信息
    //再根据群组信息，找的该群组下所有用户的userid,并且通user查出用户的详细信息
    char sql[1024]={0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from allgroup a inner join \
         groupuser b on a.id = b.groupid where b.userid=%d",
            userid);
    vector<Group>groupVec;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if(res !=nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res))!= nullptr)
            {
                Group group;
                group.setid(atoi(row[0]));
                group.setname(row[1]);
                group.setdesc(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
            
        }
    }
    //从每个小群组里面得到用户信息User表中存到vector<Groupuser>里面
    for(Group &group : groupVec)
    {
       sprintf(sql, "select a.id,a.name,a.state,b.grouprole from user a \
            inner join groupuser b on b.userid = a.id where b.groupid=%d",
                group.getid());
        MYSQL_RES *res = mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res))!=nullptr)
            {
                Groupuser user;
                user.setid(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return groupVec;

}
//根据群组用户id列表，给群组的其他成员发消息
vector<int> GroupModel::queryGroupUsers(int userid ,int groupid)
{
    char sql[1024] = {0};
    sprintf(sql,"select userid from groupuser where groupid = %d and userid !=%d" , groupid,userid);
    vector<int>idVec;     // 存放查找到的userid
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res))!=nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idVec;
}   