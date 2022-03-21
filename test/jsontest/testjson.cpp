#include"json.hpp"


#include<iostream>
using json = nlohmann::json;
using namespace std;

//json序列化示例
/*void func1(){
    json js;
    js["name"] = "maoning";
    js["nianling"] = 24;
    js["xingbie"] = "nan";
   // cout <<js<<endl;
    string s=js.dump();
    cout <<s.c_str()<<endl;
}*/
//json序列化容器
#include<vector>
#include<map>
string  func3(){
    json js;
    vector<int> v={1,2,3,4,5};
    js["id"] = v;
    map<int ,string>m;
    m.insert({1,"黄山"});
    m.insert({2,"泰山"});
    m.insert({3,"华山"});
    js["path"]=m;
    string sendbuf = js.dump();
     cout<<sendbuf<<endl;
    return sendbuf;
 
}
#include<map>
int main(){
  

   string recvbuf = func3();
   json jsbuf = json::parse(recvbuf);
    cout<<jsbuf["path"]<<endl;
    
    auto arr = jsbuf["path"];
    cout<< arr[2]<<endl;
    map<int ,string >m = jsbuf["path"];
    for(auto &p :m){
        cout<<p.first<<"  "<<p.second;
    }
    cout<<endl;
    return 0;
}