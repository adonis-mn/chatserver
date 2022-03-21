#ifndef GROUPUSER_H
#define GROUPUSER_H
#include"user.hpp"

//相比于user多一个role角色信息
class Groupuser :  public User
{
public:
    void setRole(string role ){ this->role = role;}
    string getRole()    {return role;}
private:
    string role;
};

#endif