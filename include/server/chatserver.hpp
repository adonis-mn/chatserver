#ifndef CHATSERVER_H
#define CHATSERVER_H

#include<muduo/net/EventLoop.h>
#include<muduo/net/TcpServer.h>

using namespace muduo;
using namespace muduo::net;

class ChatServer
{
public:
    ChatServer(EventLoop *loop ,const InetAddress& listenaddr , const string& name);
    void start();
private:
    TcpServer _server;
    EventLoop *_loop;
    void oneconnection(const TcpConnectionPtr &conn);
    void onemessage(const TcpConnectionPtr &conn,  Buffer* buffer , Timestamp time );

};

#endif