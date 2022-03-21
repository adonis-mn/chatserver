#ifndef GROUPMODEL_H
#define GRoupMODEL_H
#include"group.hpp"

class GroupModel
{ 
public: 
    //创建群聊
    bool createGroup(Group& group);
    //加入群聊
    void addGroup(int userid,int groupid,string role);
    //查询用户所在群组信息
    vector<Group> queryGroups(int userid);
    //获取用户所在群组其他用户id列表
    vector<int> queryGroupUsers(int userid ,int groupid);
private:
    
}; 

#endif