#ifndef USERMODEL_H
#define USERMODEL_H
//这个类主要是在对user表的数据进行操作
#include"user.hpp"
class UserModel
{
public:
        bool insert(User& user);
        User  query(int id);
        bool updatestate(User& user);
        void resetState();
private:
};





#endif