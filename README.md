# chatRoom
基于手写muduo网络库实现的集群聊天服务器和客户端代码，网络框架是经典的one thread one loop框架，并通过nginx实现tcp负载均衡、还通过redis消息订阅实现跨服务器聊天。
