
#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"

#include <functional>
#include <string>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
{
    // 注冊连接回调
    _server.setConnectionCallback(bind(&ChatServer::onConnect, this, _1));

    // 注册读写事件回调
    _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置线程数量
    _server.setThreadNum(2);
}

void ChatServer::start()
{
    _server.start();
}

// 上报连接相关的回调
/*
    客户端下线的正常流程，正常流程
*/
void ChatServer::onConnect(const TcpConnectionPtr &conn)
{
    //客户端断开连接了
    if(!conn->connected()) 
    {
        ChatService::instance()->clientCloseException(conn); //客户端异常关闭了
        conn->shutdown();
    }
}
// 上报读写事件相关信息回调的函数
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time)
{
    string buf = buffer->retrieveAllAsString();  //包真的一次收全了吗
    //数据反序列化
    json js = json::parse(buf);
    // 完全解耦网络模块的代码和业务模块的代码，不要指明道姓的调用
    // 通过js["msgid"]绑定一个回调操作，通过js["msgid"]获取业务处理函数
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>()); //js["msgid"].get<int>() 强制讲msgid对应的值转成整型
    // 回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(conn,js,time);
}