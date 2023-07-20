#include "chatservice.hpp"
#include "public.hpp"
#include <string>
#include <muduo/base/Logging.h> //日志
#include <vector>
#include <map>
using namespace muduo;
using namespace std;
// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service; // 线程安全的
    return &service;
}

// 注册消息已经对应的handler回调操作
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});

    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    // 连接redis服务器
    if (_redis.connect())
    {
        // 设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志，msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // LOG_ERROR << "msgid:"<<msgid <<"con not find handler!";
        // 返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp)
        {
            LOG_ERROR << "msgid:" << msgid << "con not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

// 服务器异常后，业务重置方法
void ChatService::reset()
{
    // 把所有用户的状态online设置为offline
    _userModel.resetState();
}

// 处理登录业务  ORM Object relation map 对象关系映射  业务层操作的都是对象
// DAO 数据层，这一层才有数据的操作
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"]; // 通过id登录？
    // int id = js["id"].get<int>()
    string pwd = js["password"];
    User user = _userModel.query(id);               // 查询id，通过id拿到对应的user数据
    if (user.getId() != -1 && user.getPwd() == pwd) // 也可以通过user.getId() == id来判断是不是真的有个用户
    {
        if (user.getState() == "online")
        {
            // 该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["erron"] = 2;
            response["errmsg"] = "this account is using, input another!";
            conn->send(response.dump());
        }
        else
        {
            // 登录成功，记录用户连接信息,可能会被多线程访问，需要考虑线程安全
            {
                lock_guard<mutex> guard(_connMutex);
                _userConnMap.insert({id, conn});
            }
            // 登录成功后，需要注册通道,通道对应的就是用户的id
            _redis.subscribe(id); // 别的服务器可能给这个用户id发送消息

            // 数据库的并发操作，是由mysql来保证的
            //  登录成功,更新用户状态信息 state offline=>online
            user.setState("online");
            _userModel.updateState(user);

            // 局部变量，线程栈之间都是隔离的
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["erron"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 查询该用户是否有离线消息，有的话就填充到json中
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 读取完离线消息之后，把该用户的所有离线消息删除吊
                _offlineMsgModel.remove(id);
            }
            // 查询该用户的好友信息并返回
            vector<User> uservec = _friendmodel.query(id); // User是自定义类型插不进json
            if (!uservec.empty())
            {
                vector<string> vec2;
                for (User &user : uservec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }
            // 查询用户的群组消息
            vector<Group> groupuserVec = _groupmodel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["sate"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }
                response["groups"] = groupV;
            }
            conn->send(response.dump());
        }
    }
    else
    {

        // 用户不存在，或者用户存在但是密码错误导致登录失败 可以继续划分
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["erron"] = 1;
        response["errmsg"] = "id or password is invalid!";
        conn->send(response.dump());
    }
}
// 处理注册业务 name password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["erron"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["erron"] = 1;
        // resopnse["errmsg"] = 1;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
}

// 处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> guard(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }
    //用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(userid);

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
}

////处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> guard(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从map表删除用户的连接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    // 用户注册，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(user.getId());

    // 更新用户的状态信息
    if (user.getId() != -1) // 防止没有找到
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

// 一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["to"].get<int>();
    {
        lock_guard<mutex> guard(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // 找到了(在线)，需要转发消息, 服务器做消息中转，主动推送消息给toid
            it->second->send(js.dump());
            return;
        }
    }

    //查找to是否在线，如果在线，就先publish给redis上的toid命名的通道上
    User user = _userModel.query(toid);
    if(user.getState()=="online")
    {
        _redis.publish(toid,js.dump());
        return;
    }

    // toid不在线，需要存储离线消息
    _offlineMsgModel.insert(toid, js.dump());
}

// 添加好友业务 msgid id friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>(); // 如果添加的这个人不存在怎么办？可以查询一下
    // 但是这里的做法是直接添加好友关系,到时候返回信息的时候会和user进行匹配
    // 存储好友信息
    _friendmodel.insert(userid, friendid);
    // 后面可以响应给客户端添加成功
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    // 存储新创建的群组信息
    Group group(-1, name, desc);
    if (_groupmodel.createGroup(group)) // 在allgroup中插入一个群，群的主键id是自动生成的
    {
        // 存储群组创建人信息
        _groupmodel.addGroup(userid, group.getId(), "creator"); // 将userid以creator的身份加入群
        // 群组信息创建成功也可以发个响应
    }
    // else 创建失败，可以扩充
}
// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupmodel.addGroup(userid, groupid, "normal"); // 将用户和群插入groupuser表中
    // 加入成功也可以发响应
}
// 群聊天
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupmodel.queryGroupUsers(userid, groupid);
    lock_guard<mutex> lock(_connMutex);
    for (int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            // 转发消息
            it->second->send(js.dump());
        }
        else
        {
            // 查询用户是否在线，如果在线，就向对应的channel上发布消息
            User user = _userModel.query(id);
            if(user.getState() == "online")
            {
                _redis.publish(id,js.dump());
            }
            else
            {
                // 存储离线消息
                _offlineMsgModel.insert(id, js.dump());
            }
            
        }
    }
}

// 从redis消息队列中获取订阅的消息, 通道为userid的消息就被转发到userid对应的服务器上来了，然后在服务器中寻找userid
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息  可能在上报的时候，也就是取消息的过程中，userid下线了
    _offlineMsgModel.insert(userid, msg);
}