#ifndef GROUP_H
#define GROUP_H

#include"groupuser.hpp"
#include<vector>

class Group
{
private:
    int id ;
    string name;            //组名称
    string desc;            //组功能
    vector<Groupuser> users;
public:
    Group(int id = -1,string name ="" ,string desc ="")
    {
        this->desc=desc;
        this->id = id ;
        this->name = name;
        
    }
    void setid(int id){ this->id =id; }
    void setname(string name ){ this->name =name; }
    void setdesc(string desc){ this->desc =desc ;}
    int  getid(){  return this->id; }
    string getname(){ return this->name ;}
    string getdesc(){ return this->desc ;}
    vector<Groupuser>&  getUsers(){return this->users;}
};

#endif