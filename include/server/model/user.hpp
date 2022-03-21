#ifndef USER_H
#define USER_H

#include<string>
using namespace std;
//用一个类去保存我的用户信息，每一个类对应一个用户信息
class User
{
public:
    User(int id =-1 ,string name =" ",string pwd =" ",string state ="offline")
    {
        this->id = id;
        this->name = name;
        this->pwd = pwd;
        this->state = state;
    }
    void setid(int id){ this->id = id;}
    void setName(string name){  this->name = name;}
    void setPwd(string pwd){ this->pwd = pwd;}
    void setState(string state){    this->state =state;}
    int getid(){return this->id;}
    string getName(){return this->name;}
    string getPwd(){    return this->pwd;}
    string getState(){  return this->state;}
private:
    int id ;
    string name;
    string  pwd;
    string state; 

};

#endif