#include"chatservice.hpp"
#include<muduo/base/Logging.h>
#include"public.hpp"
#include"user.hpp"
#include<mutex>      //lock_guard类 在这个头文件中
//using namespace std;
//获取单例对象的接口函数
ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}
//注册消息以及对应的handler回调操作
ChatService:: ChatService()
{
   //将login/reg bind到msghandler上
    _msgHandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::login ,this,_1,_2,_3)});   
    _msgHandlerMap.insert({REG_MSG,std::bind(&ChatService::reg ,this,_1,_2,_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG,std::bind(&ChatService::oneChat,this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addFriend,this,_1,_2,_3)});

    //群组业务事件处理回调
    _msgHandlerMap.insert({CREATE_GROUP_MSG,std::bind(&ChatService::createGroup,this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG,std::bind(&ChatService::addGroup,this,_1,_2,_3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG,std::bind(&ChatService::groupChat,this,_1,_2,_3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});

    //业务初始化的时候，连接redis
    if(_redis.connect())
    {
        //设置redis订阅之后上报消息回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage ,this ,_1, _2));
    }
}

void ChatService::login(const TcpConnectionPtr&conn ,json& js,Timestamp time)
{
    //登录业务
    //要将业务模块代码跟数据模块代码分开，在这里不要出现对数据库的增删改查。
    //LOG_INFO << "do login service !!!";
    int  id = js["id"].get<int>();
    string pwd = js["password"];
    User user = _usermodel.query(id);
    if(user.getid() == id && user.getPwd() ==  pwd)
    {
        
        if(user.getState()== "online")
        {
            //说明用户已经登录，这里不允许重复登录
        json response;
        response["msgid"]= LOGIN_MSG_ACK;
        response["error"] =2 ;   //0代表成功，2代表重复登录，1代表登录失败
        response["errmsg"] = "this accout is using ,input another!" ;  
        conn->send(response.dump());
        }
        else
        {
        //用哈希表来存储在线用户的通信连接，并实现线程同步
        {
        lock_guard<mutex> lock(_connMutex);      //对连接信息表进行上锁，
        _userConnMap.insert({id,conn});          //对于mysql的并发操作是通过mysqlserver来解决的，我们不需要考虑
        }

        //登录成功之后，向以id为通道，向redis订阅通道
        _redis.subscribe(id);

        json response;
        user.setState("online");
        _usermodel.updatestate(user);    // 将user里面的state更新为online
        response["msgid"]= LOGIN_MSG_ACK;
        response["error"] =0 ;   //0代表成功
        response["id"]  = user.getid();
        response["name"] = user.getName();

        //登录成功了查看是否有离线数据
         vector<string> vec = _offlineMsgModel.query(id);
        if (!vec.empty())
        {
                response["offlinemsg"] = vec;
                // 读取该用户的离线消息后，把该用户的所有离线消息删除掉
                _offlineMsgModel.remove(id);
        }

        //返回好友列表
        vector<User>uservec = _friendModel.query(id);
        if(!uservec.empty())
        {
            vector<string>vec2;
            for( User& user : uservec)
            {
                json js;
                js["id"] = user.getid();
                js["name"] = user.getName();
                js["state"] = user.getState();
                vec2.push_back(js.dump());
            }
            response["friends"] =vec2;

        }
        
        //查询用户的群组信息,包括群组里的好友信息
        vector<Group>groupvec = _groupModel.queryGroups(id);
        if(!groupvec.empty())
        {
            vector<string>groupV;
            for(Group& group : groupvec)
            {
                json grpjson;       //群组信息
                grpjson["id"] = group.getid();
                grpjson["groupname"] = group.getname();
                grpjson["groupdesc"]  = group.getdesc();
                vector<string>userV;
                for(Groupuser& user : group.getUsers())
                {
                    json js;        //好友信息
                    js["id"] = user.getid();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    js["role"] = user.getRole();
                    userV.push_back(js.dump());
                }
                grpjson["users"] = userV;
                groupV.push_back(grpjson.dump());
            }
            response["groups"] = groupV;
        }

        conn->send(response.dump());
        }
    }
    else
    {
        //登陆失败,密码错误/用户不存在
        json response;
        response["msgid"]= LOGIN_MSG_ACK;
        response["error"] =1 ;   //0代表成功
        response["errmsg"]="id or password is invalid!";
        conn->send(response.dump());
    }
}
//注册业务
void ChatService::reg(const TcpConnectionPtr&conn ,json& js,Timestamp time)
{
    LOG_INFO << "do reg service !!!"; 
    User user; 
    string name =js["name"];
    string password = js["password"];
    user.setName(name);
    user.setPwd(password);
    bool state = _usermodel.insert(user);     //在这一步向mysql插入数据的时候表的时候，完成mysql的连接，实现完全解耦
    if(state)
    {
        //插入成功
        json response;
        response["msgid"]= REG_MSG_ACK;
        response["error"] =0 ;   //0代表成功
        response["id"]  = user.getid();
        conn->send(response.dump());
    }
    else
    {
        //插入失败

        json response;
        response["msgid"]= REG_MSG_ACK;
        response["error"] =1 ;   //0代表成功
       // response["id"]  = user.getid();
        conn->send(response.dump());
    }
    
}

//处理在线用户异常退出
void ChatService::clientCloseException(const TcpConnectionPtr& conn)
{
    //将 在线用户通信连接表中的在线连接数据删除，并将mysql中user表中state改为offline
    lock_guard<mutex> lock(_connMutex);
    User user;
    for(auto it =_userConnMap.begin();it!=_userConnMap.end();it++)
    {
        if(it->second == conn)
        {
            user.setid(it->first);
            _userConnMap.erase(it);
            break;
        }
    }
    //取消当前用户的redis的订阅通道
    _redis.unsubscribe(user.getid());
    
    //更新user-state信息
    if(user.getid() != -1)
    {
        user.setState("offline");
        _usermodel.updatestate(user);
    }
}

//注销业务
void ChatService::loginout(const TcpConnectionPtr&conn ,json& js, Timestamp time)
{
   int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    // 用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(userid); 

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    _usermodel.updatestate(user);
}

//一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr&conn ,json& js ,Timestamp time)
{
    int toid = js["to"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it =_userConnMap.find(toid);
        if(it != _userConnMap.end())
        {
            //toid在线 ，转发消息，服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return ;          //在这里释放锁，锁的力度要尽量小
        }
    }
    //查询to是否在线
    if( _usermodel.query(toid).getState() =="online"  )
    {
        _redis.publish(toid,js.dump());
        return;
    }

    //toid不在线,存入离线消息
    _offlineMsgModel.insert(toid,js.dump());

}

//服务器异常断开后，重设用户状态
void ChatService::reset()
{
    _usermodel.resetState();
}

//添加好友
void ChatService::addFriend(const TcpConnectionPtr&conn , json& js , Timestamp time)
{   
    int friendid = js["friendid"].get<int>();
    int userid = js["id"].get<int>();
    _friendModel.insert(userid ,friendid);
}

//创建群组业务
void ChatService::createGroup(const TcpConnectionPtr&conn , json& js , Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    Group group(-1,name,desc);
    if(_groupModel.createGroup(group))
    {
        //存储群组创建人信息
        _groupModel.addGroup(userid,group.getid(),"creator");
    }

}

//加入群聊业务
void ChatService::addGroup(const TcpConnectionPtr&conn , json& js , Timestamp time)
{
    int userid =js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid,groupid,"normal");
}

//群聊业务
void ChatService::groupChat(const TcpConnectionPtr&conn , json& js , Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
    lock_guard<mutex> lock(_connMutex);
    for(int id : useridVec)
    {
        auto it =_userConnMap.find(id);
        if(it != _userConnMap.end())
        {
            //转发消息
            it->second->send(js.dump());
        }
        else 
        {
            User user = _usermodel.query(id);
            if(user.getState() == "online")
            {
                _redis.publish(id , js.dump());
                return ;
            }
            //存储离线群消息
            _offlineMsgModel.insert(userid,js.dump());
        }
        
    }
}

//从redis消息队列中获取订阅消息
void ChatService::handleRedisSubscribeMessage(int userid ,string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if(it != _userConnMap.end())
    {
        it->second->send(msg);
        return ;
    }
    // 存储该用户的离线消息
    _offlineMsgModel.insert(userid, msg);
}