#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
using namespace std;

/*
redis作为集群服务器通信的基于发布-订阅消息队列时，会遇到两个难搞的bug问题，参考我的博客详细描述：
https://blog.csdn.net/QIANGWEIYUAN/article/details/97895611
*/
class Redis
{
public:
    Redis();
    ~Redis();

    // 连接redis服务器 
    bool connect();

    // 向redis指定的通道channel发布消息
    bool publish(int channel, string message); //向channel通道发布消息message

    // 向redis指定的通道subscribe订阅消息
    bool subscribe(int channel);  //对哪个通道的消息感兴趣

    // 向redis指定的通道unsubscribe取消订阅消息
    bool unsubscribe(int channel);  //用户下线了，就不需要再订阅了，其他人发的消息就是离线消息了

    // 在独立线程中接收订阅通道中的消息
    void observer_channel_message();  //独立线程，专门接收通道的消息

    // 初始化向业务层上报通道消息的回调对象
    void init_notify_handler(function<void(int, string)> fn); //设置通知回调

private:
    // hiredis同步上下文对象，负责publish消息
    redisContext *_publish_context;  //redis提供的类 redisContext，相当于redis-cli连接之后的上下文信息
    /*
        subscribe 之后 一个redis-cli上下午会被阻塞，所以需要一个publish和一个subscribe的上下文
        也就是说，发布消息要在一个上下文，订阅一个消息会在另外一个上下文
    */

    // hiredis同步上下文对象，负责subscribe消息   
    redisContext *_subcribe_context;

    // 回调操作，收到订阅的消息，给service层上报    订阅一个消息之后，如果对应的channel有事件发生了，就会通过这个回调函数通知 
    // 收到的消息，有一个message标头，再就是通道号，最后是消息具体内存
    /*
    "message"
    "13"
    "hello world"
    */
    function<void(int, string)> _notify_message_handler;  // int 对应的通道号，string是具体的内容
};

#endif
