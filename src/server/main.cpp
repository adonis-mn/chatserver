#include"chatserver.hpp"
#include"chatservice.hpp"
#include<iostream>
#include<signal.h>
using namespace std;
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);   //退出程序
}
int main(int  argc ,char** argv)
{
    if(argc <3)
    {
        cerr<<"command invalid! example : /ChatServer 127.0.0.1 6000" <<endl;
        exit(-1);
    }
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);
    signal(SIGINT,resetHandler);  //SIGINT程序终止(interrupt)信号, 在用户键入INTR字符(通常是Ctrl+C)时发出，用于通知前台进程组终止进程。
    EventLoop loop;
    InetAddress addr(ip,port);
    ChatServer server(&loop,addr,"ChatServer");
    server.start();
    loop.loop();
    return 0;
}