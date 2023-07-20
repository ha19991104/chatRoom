/*
    muduo网络库给用户提供了两个主要的类
    TcpServer: 用于编写服务器程序
    TcpCLient: 用于编写客户端程序
TcpServer封装之后，可以很好将网络IO的代码和业务代码区分开，指向提供用户连接和断开  用户可读写事件的 接口
*/
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<string>
#include<functional>
#include<iostream>
using namespace std;
using namespace muduo;
using namespace muduo::net;
// 基于muduo网络库开放服务器程序
/*
1.组合TcpServer对象，在TcpServer外层就只需要关注业务代码
2.创建EventLoop事件循环对象的指针
3.明确TcpServer构造函数需要什么参数
4.在当前服务器类的构造函数当中，注册处理连接的回调函数和处理读写事件的回调函数
5.设置合适的服务端线程数量，muduo库会自己分配
*/
class chatServer
{
public:
    chatServer(EventLoop* loop,  //事件循环
            const InetAddress& listenAddr, //IP + Port
            const string& nameArg) //服务器的名字
            :server_(loop,listenAddr,nameArg)
            ,loop_(loop_)
            {
                //给服务器注册用户连接的创建和断开回调
                server_.setConnectionCallback(std::bind(&chatServer::onConnetion,this,std::placeholders::_1));
                //给服务器注册用户读写事件回调
                server_.setMessageCallback(std::bind(&chatServer::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
                //设置服务器端的线程数量 1个主线程 3工作线程
                server_.setThreadNum(3);
            
            }
            void start()
            {
                server_.start();
            }
private:
    //专门用于处理连接的连接和断开
    void onConnetion(const TcpConnectionPtr& conn)
    {
       
        if(conn->connected())
        {
             std::cout<< conn->peerAddress().toIpPort()
                        <<"-->"
                        <<conn->localAddress().toIpPort()
                        <<" status: online" <<std::endl;
        }
        else
        {
             std::cout<< conn->peerAddress().toIpPort()
                        <<"-->"
                        <<conn->localAddress().toIpPort()
                        <<" status: offline" <<std::endl;
            conn->shutdown(); // ?这个是有必要的吗
        }
    }

    //专门处理用户的读写事件
    void onMessage(const TcpConnectionPtr& conn/*连接*/,Buffer *buf/*缓冲区*/,Timestamp time/*接收到数据的时间*/)
    {
        string msg = buf->retrieveAllAsString();
        cout<<"recv data: "<<msg <<"time "<<time.toString()<<endl;
        conn->send(msg);
    }
    muduo::net::TcpServer server_; // #1
    muduo::net::EventLoop *loop_;
};

int main()
{
    /*
        g++ -o muduo_server muduo_server.cpp -lmuduo_net -lmuduo_base -lpthread -std=c++11 -g
    */
    EventLoop mainloop;
    InetAddress addr("0.0.0.0",8888);
    chatServer server(&mainloop,addr,"chatServer");
    server.start(); //开启线程，挂载listenfd到epoll上
    mainloop.loop(); //开主线程循环，listenfd等待新用户连接

    return 0;
}