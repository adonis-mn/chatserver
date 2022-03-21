//muduo网络库给用户提供了两个主要类
//TcpServer ：编写服务器程序
//TcpClient:  用于编写客户端程序 

#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
using namespace std;
#include<iostream>
using namespace muduo;
using namespace muduo::net;
//  1.复合TcpServer对象
//  2.在构造函数中对TcpServer对象初始化
//  3.构造函数中注册回调函数
//  4.在回调函数中，处理连接，和读写事件，并输出ip和端口连接是否成功，读写的数据

class ChatServer
{
public:        //事件循环， IP+PORT,  服务器名字
    ChatServer(EventLoop *loop,const InetAddress& listenAddr,const string &nameArg):_server(loop,listenAddr,nameArg),_loop(loop){
        //给服务器用户连接的创建，断开回溯
        _server.setConnectionCallback(std::bind(&ChatServer::oneConnection,this,_1));
        //给服务器注册用户读写事件回调
        _server.setMessageCallback(std::bind(&ChatServer::oneMessage,this,_1,_2,_3));
        
        _server.setThreadNum(4);
    }
    void start()
    {
        _server.start();
    }
        
private:
    //专门用来处理用户的连接创建和断开    epoll accept  listen
    void oneConnection(const TcpConnectionPtr& conn)
    {
        //连接创建，断开
        if(conn->connected()){
            //连接成功
            cout<<conn->peerAddress().toIpPort()<<" -> "<<conn->localAddress().toIpPort()<<"state: online" <<endl;
        }
        else 
        {
            cout<<conn->peerAddress().toIpPort()<<" -> "<<conn->localAddress().toIpPort()<<"state: offline" <<endl;
        }
    }
    //参数 连接  ， 缓冲区  ， 接收到数据的时间信息
    void oneMessage(const TcpConnectionPtr& conn ,Buffer *buffer ,Timestamp time)
    {
        string buf = buffer->retrieveAllAsString();
        cout<<"recv data:"<<buf <<" time:"<<time.toString()<<endl;
        conn->send(buf);
    }

    TcpServer _server;
    EventLoop *_loop;    //可以看作epoll事件循环
};

int main()
{
    EventLoop loop;
    InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop,addr,"chatserver");
    server.start();    //listen   epoll_ctl
    loop.loop();       //epoll_wait以堵塞方式等待新用户连接，已连接用户的读写事件
    return 0;
}
//之后在编译时需要自己去连接库 -muduo_net -muduo_base -lpthread