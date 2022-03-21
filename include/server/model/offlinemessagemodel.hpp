#ifndef OFFLINEMSGMODEL_H
#define OFFLINEMSGMODEL_H
#include<vector>
#include<string>
using namespace std;   //vector在std命名空间下
class OfflineMsgModel
{

    
public:
   vector<string> query(int userid);

   void remove(int userid);

   void insert(int userid,string msg);
   
};
   
#endif


