
#include<pthread.h>
#include<thread>
#include<unistd.h>
#include"json.hpp"
#include<sys/socket.h>
#include<arpa/inet.h>
#include<vector>
#include"user.hpp"
#include"group.hpp"
#include"public.hpp"
#include<iostream>

using namespace std;
using json = nlohmann::json;


//记录当前系统登录的用户信息
User g_currentUser;
//记录当前登录用户的好友列表
vector<User> g_currentUserFriendList;
//记录当前登录用户的群组列表信息
vector<Group> g_currentUserGroupList;

//控制主菜单页面程序
bool isMainMenuRunning = false;

//显示当前登录成功用户的基本信息
void showCurrentUserData();

//客户端只是接收跟发送，不需要高并发
//接收线程
void readTaskHandler(int clientfd);

//获取系统的时间（聊天信息也需要添加时间信息
string getCurrentTime();

//主聊天页面程序
void mainMenu(int clientfd);

//聊天客户端程序的实现，main线程用作发送线程，子线程用作接受线程
int main(int argc ,char** argv)
{
    if(argc<3)
    {
        cerr<<"command invalid example: ./chatclient  127.0.0.1 6000"<<endl;
        exit(-1);      //exit(0)表示程序正常退出，-1，1都代表异常退出
        //cerr用于输出错误，无缓存 ，cout带缓存区，标准输出
    }
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);
    int clientfd = socket(AF_INET , SOCK_STREAM , 0);
    if(clientfd == -1)
    {
        cerr<<"socket create error" <<endl;
        exit(-1);
    }
    //填写client需要连接的server的ip+port
    sockaddr_in  server;
    memset(&server,0 , sizeof(sockaddr_in)) ; 
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    
    //client 和 server进行连接
    if(connect(clientfd , (sockaddr*)&server  , sizeof(sockaddr_in))==-1)
    {
        cerr<<"connect server error" <<endl;
        close(clientfd);
        exit(-1);
    }
    
    //连接成功之后，main线程用于接受用户输入，负责发送数据
    for(;;)
    {
        cout<<"==========================="<<endl;
        cout<<"1.login"<<endl;
        cout<<"2.register"<<endl;
        cout<<"3.quit"<<endl;
        cout<<"==========================="<<endl;
        cout<<"choice:" ;
        int choice =0 ;
        cin>>choice;
        cin.get();        //一般输入完我们都会按回车，为了不影响下次的写，读掉缓存区残留的回车

        switch (choice)
        {
        case 1:           //login
        {
            int id = 0;
            char pwd[50] = {0};
            cout << "userid:";
            cin >> id;
            cin.get(); // 读掉缓冲区残留的回车
            cout << "userpassword:";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();

            int len =send(clientfd,request.c_str(),strlen(request.c_str())+1 , 0 ) ;  //字符串最后还有一个 /'0'来结束，占一个大小
            if(len == -1)
            {
                cerr<<"send reg msg error"<<endl;
            }
            else
            {
                //发送完给服务器数据之后，读来自服务器的响应信息
                char buf[1024] = {0};
                len = recv(clientfd , buf, 1024,0);             //堵塞
                if(len == -1)
                {
                    cerr<<"recv reg response error"<<request<<endl;
                }
                else
                {
                    //读到了数据，将buf 反序列化
                    json  response = json::parse(buf);
                    //根据服务器返回的数据，判断是否注册成功 
                    if(response["error"].get<int>() != 0)
                    {
                        cout<<response["errmsg"] <<endl;
                    }               
                    else
                    {
                        //记录当前用户的id,name
                        g_currentUser.setid(response["id"].get<int>());
                        g_currentUser.setName(response["name"]);

                        //记录好友的列表信息
                        if(response.contains("friends"))     //contain()用来查找
                        {
                            vector<string>vec = response["friends"];
                            g_currentUserFriendList.clear();
                            for(auto str:vec)
                            {
                              json js = json::parse(str);
                              User user;
                              user.setid(js["id"].get<int>()) ;
                              user.setName(js["name"]);
                              user.setState(js["state"]);
                              g_currentUserFriendList.push_back(user);
                            }
                        }

                        //记录当前用户的群组列表信息
                        if(response.contains("groups"))
                        {
                            vector<string>vec1 = response["groups"];
                             g_currentUserGroupList.clear();
                            for(string &str:vec1)
                            {
                                json grpjs = json::parse(str);
                                Group group;
                                group.setid(grpjs["id"].get<int>());
                                group.setname(grpjs["groupname"]);
                                group.setdesc(grpjs["groupdesc"]);
                                vector<string>vec2 = grpjs["users"];
                               
                                for(string &userstr:vec2)
                                {
                                    json js = json::parse(userstr);
                                    Groupuser user;
                                    user.setid(js["id"].get<int>());
                                    user.setName(js["name"]);
                                    user.setRole(js["role"]);
                                    user.setState(js["state"]);
                                    group.getUsers().push_back(user);
                                }
                                g_currentUserGroupList.push_back(group);
                            }
                        }

                        //显示用户的基本信息
                        showCurrentUserData();

                        //显示当前用户的离线信息，个人聊天信息或者群组信息
                        if(response.contains("offlinemsg"))
                        {
                            vector<string>vec = response["offlinemsg"];
                            for(string &str : vec)
                            {
                                //个人消息和群组消息
                                json js = json::parse(str);
                                if(js["msgid"].get<int>()  == ONE_CHAT_MSG)
                                {
                                    cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                                     << " said: " << js["msg"].get<string>() << endl;
                                }
                                else
                                {
                                   cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                                    << " said: " << js["msg"].get<string>() << endl;
                                }
                            }

                        }
                        //主线程用来接收用户输入的命令，如果用户不输入，就会堵塞在这，无法recv接收服务器的消息，我们需要一个副线程来recv
                        //登录成功，启动接收线程来接收数据；
                        static int readthreadnumber = 0;
                        if(readthreadnumber == 0)
                        {
                        std::thread readTask(readTaskHandler,clientfd);
                        readTask.detach();
                        readthreadnumber++;
                        }
                        //进入聊天主菜单页面
                        isMainMenuRunning = true;
                        mainMenu(clientfd);
                    }
                }
            
            }
            
        }
        break;
        case 2:           //register业务
        {
            char name[50] = {0};
            char password[50] = {0};
            cout<<"username:";
            cin.getline(name,50);
            cout<<"userpassword:";
            cin.getline(password,50);

            json js;
            js["msgid"]  = REG_MSG;
            js["name"] = name;
            js["password"] = password;
            string request = js.dump();
            
            int len = send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
            if(len == -1)
            {
                cerr<<"send reg msg error"<<endl;
            }
            else
            {
                char buffer[1024] = {0};
                len = recv(clientfd,buffer,1024,0);
                if(len == -1)
                {
                    cerr<<"recv reg response error"<<endl;
                }
                else
                {
                    json responsejs = json::parse(buffer);
                    if((responsejs["error"].get<int>())!= 0)
                    {
                        cerr<<name <<"is alread exist ,register error!" <<endl;
                    }
                    else
                    {
                        cout<<name<< "register success, userid is" << responsejs["id"]<<", do not forget it!" <<endl;
                    }
                }
            
            }
        
        }
        break;
        case 3:           //quit
        {
            close(clientfd);
            exit(0);
        }
        default:
            cerr<<"invalid input!"<<endl;
            break;
        }
    }
    return 0;
}
//用于读写进程之间通信的
//sem_t rwsem;

void readTaskHandler(int clientfd)
{
   for(;;)
   {
       char buffer[1024]={0};
       int len = recv(clientfd,buffer,1024, 0);
       if(len == -1 || len==0)   //recv <0 表示出错   ==0 表示socket连接关闭  >0 表示接受到数据的大小
       {
           close(clientfd);
           exit(-1);
       }
       json js = json::parse(buffer);
       int msgtype = js["msgid"].get<int>();
       //time + id + name +said: +xxxx
       if(msgtype == ONE_CHAT_MSG)
       {
           cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
           continue;
       }
       if(msgtype == GROUP_CHAT_MSG)
       {
             cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
            continue;
       }
       
   }
}

void showCurrentUserData()
{
    cout << "======================login user======================" << endl;
    cout << "current login user => id:" << g_currentUser.getid() << " name:" << g_currentUser.getName() << endl;
    cout << "----------------------friend list---------------------" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            cout << user.getid() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "----------------------group list----------------------" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group &group : g_currentUserGroupList)
        {
            cout << group.getid() << " " << group.getname() << " " << group.getdesc() << endl;
            for (Groupuser &user : group.getUsers())
            {
                cout << user.getid() << " " << user.getName() << " " << user.getState()
                     << " " << user.getRole() << endl;
            }
        }
    }
    cout << "======================================================" << endl;
}


// "help" command handler
void help(int fd = 0, string str = "");
// "chat" command handler
void chat(int, string);
// "addfriend" command handler
void addfriend(int, string);
// "creategroup" command handler
void creategroup(int, string);
// "addgroup" command handler
void addgroup(int, string);
// "groupchat" command handler
void groupchat(int, string);
// "loginout" command handler
void loginout(int, string);

//系统支持的客户端命令列表
unordered_map<string , string> commandMap = {
    {"help" , "显示所有支持的命令,格式help"},
    {"chat", "一对一聊天,格式chat:friendid:message"},
    {"addfriend", "添加好友,格式addfriend:friendid"},
    {"creategroup", "创建群组,格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组,格式addgroup:groupid"},
    {"groupchat", "群聊,格式groupchat:groupid:message"},
    {"loginout", "注销,格式loginout"}
};

//系统支持的客户端命令处理
unordered_map<string ,function<void (int ,string)>> commandHandlerMap = {
    {"help" ,help},
    {"chat" , chat},
    {"addfriend" , addfriend},
    {"addgroup" , addgroup},
    {"groupchat" , groupchat},
    {"loginout" , loginout}
};


void mainMenu(int clientfd)
{
    help();

    char buffer[1024] = {0};
    while(isMainMenuRunning)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command; // 存储命令
        int idx = commandbuf.find(":");
        if (-1 == idx)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "invalid input command!" << endl;
            continue;
        }

        // 调用相应命令的事件处理回调，mainMenu对修改封闭，添加新功能不需要修改该函数
        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx)); // 调用命令处理方法
    }
}

//"help" command handler
void help(int ,string)
{
    cout<<"show command list >>>"<<endl;
    for(auto &p : commandMap)
    {
        cout<<p.first <<":"<< p.second<<endl;
    }
    cout<<endl;
}

//"addfrien" command handler
void addfriend(int cliendfd ,string str)
{
    int friendid  = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] =  g_currentUser.getid();
    js["friend"] = friendid;
    string buffer = js.dump();
    int len = send(cliendfd, buffer.c_str(),strlen(buffer.c_str())+1 , 0);
    if(len == -1)
    {
        cerr<<"send addfriend msg error"<<endl;
    }
}

void chat(int  clientfd , string str)
{
    int idx = str.find(":");
    if(idx == -1)
    {
        cerr<<"chat command invalid!"<<endl;
    }
    int  friendid = atoi(str.substr(0,idx).c_str());
    string message = str.substr(idx+1,str.size()-idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getid();
    js["name"] = g_currentUser.getName();
    js["to"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();          //js序列化，在这个地方出错了
    
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if(len == -1)
    {
        cerr<<"send chat msg error "<<endl;
    }
}

void creategroup(int clientfd , string str)
{
    int idx = str.find(":");
    if(idx == -1)
    {
        cerr<<"creatgroup command invalid!"<<endl;
        return ;
    }
    string  groupname = str.substr(0,idx);
    string groupdesc = str.substr(idx+1,str.size()-idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getid();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer = js.dump();
    
    int len = send(clientfd , buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len == -1)
    {
        cerr<<"send creatgroup msg error "<<endl;
    }

}

void addgroup(int clientfd ,string str)
{
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getid();
    js["groupid"] = groupid;
    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len == -1)
    {
        cerr<<"send addgroup msg error"  <<endl;
    }
}

void groupchat (int clientfd ,string str) 
{
    int idx = str.find(":");
    if(idx == -1)
    {
        cerr<<"groupchat command invalid!" <<endl;
        return ;
    }
    int groupid = atoi(str.substr(0,idx).c_str());
    string groupmsg = str.substr(idx+1,str.size()-idx);
    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getid();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = groupmsg;
    js["time"] = getCurrentTime();
    string buffer = js.dump();
    int len = send(clientfd , buffer.c_str(),strlen(buffer.c_str())+1 , 0);
    if(len == -1)
    {
        cerr<<"send groupchat msg error" <<endl;
    }
}

void loginout(int clientfd , string str)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getid();
    string buffer = js.dump();
    int len = send(clientfd , buffer.c_str(),strlen(buffer.c_str())+1 , 0 );
    if(len == -1 )
    {
        cerr<<"loginout error "<<endl;
    }
    else
    {
        isMainMenuRunning = false;
    }
}

string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}


