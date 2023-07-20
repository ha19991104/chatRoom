#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

// 聊天服務器的主类
class ChatServer
{
public:
    // 初始化聊天服务器对象
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg);
    // 启动服务
    void start();

private:
    // 上报连接相关的回调
    void onConnect(const TcpConnectionPtr &);
    // 上报读写事件相关信息回调的函数
    void onMessage(const TcpConnectionPtr &,
                   Buffer *,
                   Timestamp);
    TcpServer _server;
    EventLoop *_loop; // 可以在合适的时候quit退出主loop的循环
};

#endif