#include "friendmodel.hpp"
#include "db.h"

// 添加好友关系
void FriendModel::insert(int userid, int friendid)
{
    // 1 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values(%d,%d),(%d,%d)", userid, friendid,friendid,userid);
    MySQL mysql;         // 创建对象
    if (mysql.connect()) // 连接数据库
    {
        mysql.update(sql); // 发送sql语句
    }
}

// 返回用户好友列表  通过friendid在user表里面查询具体信息，
// 返回两个表的联合查询
vector<User> FriendModel::query(int userid)
{
    // 1 组装sql语句 
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.name,a.state from user a inner join friend b on b.friendid = a.id where b.userid=%d", userid);
    /*
    别名a表示user表，别名b表示friend表，from中指定了要查询的表示分别是user和friend
    inner jion右侧指定了要加入的表是friend，使用on将friend表中的friendid和user表中的id进行匹配，查询就可以将每个好友的详细信息和其所属的用户联系起来
    where语句中，b.userid=13限制查询结果，只返回friend表中userid为13的记录。这样查询结果只包含userid为13的用户的好友信息
    */
    vector<User> vec;
    MySQL mysql;         // 创建对象
    if (mysql.connect()) // 连接数据库
    {
        MYSQL_RES *res = mysql.query(sql); //  mysql_use_result(_conn); 不应该只保存一条数据吗
        if (res != nullptr)
        {
            // 把userid用户的所有好友信息放入vec中返回
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) // 会一直拿到为空
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}
