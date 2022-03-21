#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include<muduo/net/TcpConnection.h>
#include<functional>
#include"json.hpp"
#include<unordered_map>
#include<iostream>
#include<muduo/base/Logging.h>

#include"redis.hpp"
#include"UserModel.hpp"
#include"offlinemessagemodel.hpp"
#include<mutex>
#include"groupmodel.hpp"
#include<friendmodel.hpp>
using namespace std;   //不加这个unordered_map识别不了
using json = nlohmann::json;        
using namespace muduo;
using namespace muduo::net;


//聊天服务器业务类，也叫服务类

//定义一个事件处理器,对于消息事件的回调
//给msgid映射一个事件回调
using MsgHandler = std::function<void(const TcpConnectionPtr& conn , json& js ,Timestamp time )> ;
//这里对象有一个实例就行了，所以我们采用单例模式
class ChatService
{
    public:
    //获取单例对象的接口函数
    static ChatService* instance();

    //用于服务器异常断开，将user中用户状态更新
    void reset();


    //处理登录业务
    void login(const TcpConnectionPtr&conn ,json& js,Timestamp time);

    //处理注册业务
    void reg(const TcpConnectionPtr&conn ,json& js ,Timestamp time);

    //处理一对一聊天业务
    void oneChat(const TcpConnectionPtr&conn ,json& js ,Timestamp time);
    
    //添加好友业务
    void addFriend(const TcpConnectionPtr&conn , json& js , Timestamp time);
    
    //创建群组业务
    void createGroup(const TcpConnectionPtr&conn ,json& js ,Timestamp time);

    //加入群组业务
    void addGroup(const TcpConnectionPtr&conn ,json& js ,Timestamp time);

    //群组聊天业务
    void groupChat(const TcpConnectionPtr&conn ,json& js ,Timestamp time);

    //注销业务
    void loginout(const TcpConnectionPtr&conn ,json& js ,Timestamp time);

    //处理在线用户异常断开
    void clientCloseException(const TcpConnectionPtr &conn); 

    //根据MSGID获取业务处理器
    MsgHandler getHandler(int msgid)
    {
        auto it =_msgHandlerMap.find(msgid);
        if(it ==_msgHandlerMap.end())
        {
            return [=](const TcpConnectionPtr& conn , json& js ,Timestamp time)
            {
                LOG_ERROR<<"msgid: "<<msgid <<"    cannot find handler!" ;
            };
        }
        else return _msgHandlerMap[msgid];
    }
    //从redis消息队列中获取订阅消息
    void handleRedisSubscribeMessage(int ,string);
    
    
    private:
    ChatService();

    //存储消息id和对应的业务处理方法
    unordered_map<int ,MsgHandler>_msgHandlerMap;

    //存储在线用户的通信连接
    unordered_map<int,TcpConnectionPtr> _userConnMap;

    //定义一个互斥锁，用于线程安全
    mutex _connMutex;

    //对offlinemessage离线信息进行操作
    OfflineMsgModel _offlineMsgModel;

    //返回好友列表类
    FriendModel _friendModel;

    //我们需要在log，reg里面对UserModel进行操作
    UserModel _usermodel;

    //对群组进行操作
    GroupModel _groupModel;

    //redis操作类
    Redis _redis;
};

#endif