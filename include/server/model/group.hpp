#ifndef GROUP_H
#define GROUP_H

#include "groupuser.hpp"
#include <string>
#include <vector>
using namespace std;

/*
CREATE TABLE `allgroup` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `groupname` varchar(50) CHARACTER SET latin1 NOT NULL,
  `groupdesc` varchar(200) CHARACTER SET latin1 DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `groupname` (`groupname`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;
*/
class Group
{
public:
    Group(int id = -1, string name = "", string desc = "")
    {
        this->id = id;
        this->name = name;
        this->desc = desc;
    }
    void setId(int id) { this->id = id; }
    void setNmae(string name) { this->name = name; }
    void setDesc(string desc) { this->desc = desc; }
    int getId() { return this->id; }
    string getName() { return this->name; }
    string getDesc() { return this->desc; }
    vector<GroupUser> &getUsers() { return this->users; }

private:
    int id;  //群id是唯一的
    string name;
    string desc;
    vector<GroupUser> users;  //一般需要知道一个组里有多少成员，所以从数据库查出来之后，就放入vector给业务层使用
};

#endif