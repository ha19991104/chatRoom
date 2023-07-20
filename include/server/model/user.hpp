#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

// User映射类
// 匹配User表的ORM类
class User
{

public:
    User(int id=-1,string name="",string pwd="",string state = "offline")
    {
        this->id_= id;
        this->name_ = name;
        this->password_ = pwd;
        this->state_ = state;
    }
    ~User()=default;

    void setId(int id){this->id_ = id;}
    void setName(string name) {this->name_ = name;}
    void setPwd(string pwd) {this->password_ = pwd;}
    void setState(string state){this->state_ = state;}

    int getId(){return id_;}
    string getName() {return name_;}
    string getPwd() {return password_;}
    string getState(){return state_;}
private:
    int id_;
    string name_;
    string password_;
    string state_;
};


#endif