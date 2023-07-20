#include "usermodel.hpp"
#include "db.h"

/*
    字段        字段类型                 字段说明       约束
    id          INT                     用户id        PRIMARY KEY、AUTO_INCREMENT
    name        VARCHAR(50)             用户名        NOT NULL,UNIQUE
    password    VARCHAR(50)             用户密码      NOT NULL
    state       ENUM('online','offline') 当前登录状态  DEFUALT 'offline'
*/

bool UserModel::insert(User &user)
{
    // 1 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name,password,state) values('%s','%s','%s')", //%s字符串  char*
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
    MySQL mysql;         // 创建对象
    if (mysql.connect()) // 连接数据库
    {
        if (mysql.update(sql)) // 发送sql语句
        {
            // 获取插入成功的用户数据生成的主键id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true; // 注册成功
        }
    }
    return false;
}

User UserModel::query(int id)
{
    // 1 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id=%d", id);
    MySQL mysql;         // 创建对象
    if (mysql.connect()) // 连接数据库
    {
        MYSQL_RES *res = mysql.query(sql); // 通过查询获得包含结果集的MYSQL_RES结构体
        if (res != nullptr)
        {
            // 查询成功
            MYSQL_ROW row = mysql_fetch_row(res); // 拿一行数据，也就是user里面的一条记录
            /*
            mysql_fetch_row没有检索的行或者出现错误，返回NULL
            unsigned int mysql_field_count(MYSQL *mysql)  返回表的列数
            */
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0])); // 0-3对应四个字段
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res); // mysql_use_result、mysql_store_result返回的MYSQL_RES*指针是指向堆内存的，需要释放？
                return user;
            }
        }
    }
    return User(); // 没找都返回一个默认的临时对象，id=-1;
}

bool UserModel::updateState(User user)
{

    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());
    MySQL mysql;         // 创建对象
    if (mysql.connect()) // 连接数据库
    {
        if (mysql.update(sql)) // 发送sql语句
        {

            return true; // 修改成功
        }
    }
    return false;
}

void UserModel::resetState()
{
    char sql[1024] = "update user set state = 'offline' where state = 'online'";
    MySQL mysql;         // 创建对象
    if (mysql.connect()) // 连接数据库
    {
        mysql.update(sql); // 发送sql语句
    }
}