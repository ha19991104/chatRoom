#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include <muduo/net/TcpConnection.h>

#include <unordered_map>
#include <functional>
#include <mutex>
using namespace std;
using namespace muduo;
using namespace muduo::net;

#include "redis.hpp"
#include "groupmodel.hpp"
#include "friendmodel.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "json.hpp"
using json = nlohmann::json;
//处理消息事件的回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn,json &js,Timestamp)>;

//聊天服务器业务类 单例模式
class ChatService
{
public:    
    // 获取单例对象的接口函数
    static ChatService* instance();
    //处理登录业务
    void login(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //处理注册业务
    void reg(const TcpConnectionPtr &conn,json &js,Timestamp time);
    // 一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn,json &js,Timestamp time);
    // 添加好友业务
    void addFriend(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //创建群组业务
    void createGroup(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //加入群组业务
    void addGroup(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //群聊天
    void groupChat(const TcpConnectionPtr &conn,json &js,Timestamp time);
    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    //处理注销业务
    void loginout(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    //服务器异常后，业务重置方法
    void reset();
    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int userid, string msg);
private:
    ChatService();
    //消息id和其对应的事件处理业务方法
    unordered_map<int,MsgHandler> _msgHandlerMap;

    //存储在线用户的通信连接  id+对应的tcpconnection对象,用户上下线都会改变这个map，需要注意线程安全
    unordered_map<int,TcpConnectionPtr> _userConnMap;

    //定义互斥锁，保证_userConnMap的线程安全
    mutex _connMutex;

    // 数据操作类对象
    UserModel _userModel;  //来对user对象进行具体的操作，返回并返回填充user的信息
    // 离线数据操作类
    OfflineMsgModel _offlineMsgModel;
    // 好友操作类
    FriendModel _friendmodel;
    // 群组操作
    GroupModel _groupmodel;

    // redis操作对象
    Redis _redis;

};

#endif