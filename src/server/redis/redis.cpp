#include "redis.hpp"
#include <iostream>
using namespace std;

/*
redis同步发布订阅
*/

Redis::Redis()
    : _publish_context(nullptr), _subcribe_context(nullptr)  //初始化上下文指针置空
{
}

Redis::~Redis()
{
    if (_publish_context != nullptr)
    {
        redisFree(_publish_context); //析构的时候，需要释放指针
    }

    if (_subcribe_context != nullptr)
    {
        redisFree(_subcribe_context);
    }
}

bool Redis::connect() //创建了两个连接
{
    // 负责publish发布消息的上下文连接
    _publish_context = redisConnect("127.0.0.1", 6379);
    if (nullptr == _publish_context)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    // 负责subscribe订阅消息的上下文连接
    _subcribe_context = redisConnect("127.0.0.1", 6379);
    if (nullptr == _subcribe_context)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    // 在单独的线程中，监听通道上的事件，有消息给业务层进行上报
    // 因为单独的_subcribe_context是阻塞的，不能让其他工作线程阻塞
    thread t([&]() {
        observer_channel_message(); //专门用来接收订阅的消息
    });
    t.detach();

    cout << "connect redis-server success!" << endl;

    return true;
}

// 向redis指定的通道channel发布消息
bool Redis::publish(int channel, string message)
{
    // 给通道publish一个命令
    redisReply *reply = (redisReply *)redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());
    /*
        redisCommand= redisAppendCommand + redisBufferWrite + redisGetReply(等待回复)
        因为publish不会阻塞所以多了一个redisGetReply
    */
    if (nullptr == reply)
    {
        cerr << "publish command failed!" << endl;
        return false;
    }
    freeReplyObject(reply); //返回值reply是一个动态生成的结构体？ 用完之后需要释放
    return true;
}

// 向redis指定的通道subscribe订阅消息，是以阻塞的方式等待消息
/*
    同步发布订阅的问题所在：
    
*/
bool Redis::subscribe(int channel)
{
    // SUBSCRIBE命令本身会造成线程阻塞等待通道里面发生消息，这里只做订阅通道，不接收通道消息
    // 通道消息的接收专门在observer_channel_message函数中的独立线程中进行
    // 只负责发送命令，不阻塞接收redis server响应消息，否则和notifyMsg线程抢占响应资源
    /*
        subscribe不能使用redisCommand，因为subcribe会阻塞当前线程，所以需要先缓存，再将命令提交上去，而且只是提交告诉redis订阅这个通道，这样子不需要阻塞等待redis响应
    */
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "SUBSCRIBE %d", channel)) //redisAppendCommand将命令缓存到本地
    {
        cerr << "subscribe command failed!" << endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done)) //redisBufferWrite将本地缓存的数据发送到redis上
        {
            cerr << "subscribe command failed!" << endl;
            return false;
        }
    }
    // redisGetReply  少了一个这个不需要阻塞等待回复，就只是单纯的提交命令

    return true;
}

// 向redis指定的通道unsubscribe取消订阅消息
bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "UNSUBSCRIBE %d", channel))
    {
        cerr << "unsubscribe command failed!" << endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done))
        {
            cerr << "unsubscribe command failed!" << endl;
            return false;
        }
    }
    return true;
}

// 在独立线程中接收订阅通道中的消息
void Redis::observer_channel_message()
{
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(this->_subcribe_context, (void **)&reply)) //从_subcribe_context上redisGetReply消息，以循环阻塞的方式等待上下文上的消息
    {
        // 订阅收到的消息是一个带三元素的数组
        /*
            通道上发生消息一次会返回3个数据 分别对应的elment的0、1、2 
            0是message，1是通道号，2是具体消息
        */
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            // 给业务层上报通道上发生的消息
            /*
            element[1] 是通道号，element[2]是具体消息
            */
            _notify_message_handler(atoi(reply->element[1]->str) , reply->element[2]->str);
        }

        freeReplyObject(reply);
    }

    cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<" << endl;
}

void Redis::init_notify_handler(function<void(int,string)> fn)
{
    this->_notify_message_handler = fn;
}