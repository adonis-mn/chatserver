# chatserver
基于muduo库实现的，利用nginx负载均衡，redis实现服务器之间通信，高度解耦的集群聊天服务器和客户端源码
编译方式：执行bulid.sh即可完成编译，在bin目录中生成ChatServer聊天服务器和ChatClient聊天客户端
1.项目内容：
使用muduo网络库实现网络模块开发，达到高并发IO服务，并将网络模块，业务模块，数据模块进行高度解耦
使用json来打包传输的数据，进行序列化和反序列化
MySQL数据库编程，使用mysql关系型数据库作为项目数据的落地存储
配置nginx的tcp的负载均衡，支持长连接，实现服务器的集群功能，提高并发量
配置基于发布-订阅的服务器中间层redis，实现集群服务器之间的通信
CMake构建编译环境，Github托管项目
2.业务内容：1.客户端新用户注册2. 客户端用户登录3. 添加好友和添加群组4. 好友聊天5. 群组聊天6. 离线消息
