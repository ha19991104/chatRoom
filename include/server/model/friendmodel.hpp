#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
  
#include "user.hpp"
#include<vector>
using namespace std;


// 维护好友信息的操作接口方法
class FriendModel
{
public:
    //添加好友关系
    void insert(int userid,int friendid);
    
    // 好友列表信息一般是记录在客户端的(用户上下线的时候，好友列表不会变)，如果都在服务器，压力会很大
    // 返回用户好友列表  通过friendid在user表里面查询具体信息，返回两个表的联合查询
    vector<User> query(int userid);

};



#endif