#ifndef GROUPUSER_H
#define GROUPUSER_H
#include "user.hpp"
class GroupUser : public User  //GroupUser首先就是普通的用户
{
public:
    void setRole(string role) { this->role = role; }
    string getRole() { return this->role; }

private:
    string role; //比普通用户多了一个在群组中的角色信息
};

#endif