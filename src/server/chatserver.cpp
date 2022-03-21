#include"chatserver.hpp"
#include<functional>   //bind函数对象绑定器
#include"json.hpp"
#include"chatservice.hpp"
using json = nlohmann::json;
using namespace std;
using namespace placeholders;   //占位符

ChatServer::ChatServer(EventLoop *loop ,const InetAddress& listenaddr , const string& name): 
    _server(loop,listenaddr,name),_loop(loop)
    {
        _server.setConnectionCallback(std::bind(&ChatServer:: oneconnection ,this,_1 ));
        _server.setMessageCallback(std::bind(&ChatServer:: onemessage , this,_1,_2,_3));
        _server.setThreadNum(4);
    }

void ChatServer::start()
{
    _server.start();
}

//上传连接相关信息
void  ChatServer::oneconnection(const TcpConnectionPtr& conn)
{
        if(!conn->connected())
        {
            //在这里处理用户异常退出的情况
            ChatService::instance()->clientCloseException(conn);
            conn->shutdown();  //连接断开回收资源
        }
}
//上次读写事件的相关信息
void ChatServer::onemessage(const TcpConnectionPtr&conn ,Buffer *buffer, Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    //反序列化
    json js = json::parse(buf);
    //通过js["msgid"]获取  --》业务handler --》conn time js
    //而不是if - else 在这里指名道姓的调用业务模块
    //目的：将网络模块代码跟业务代码分开，不要去调用业务代码如（logal，req）完全解耦
    auto msghandler = ChatService::instance()->getHandler(js["msgid"].get<int>());   //js["msgid"]是json类型，通过get方法转化为int类型 
    //回调绑定好的消息处理器，来执行相应的业务处理
    msghandler(conn ,js,time);
}