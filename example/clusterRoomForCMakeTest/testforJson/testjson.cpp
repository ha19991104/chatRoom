// 序列化演示
#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
#include <string>
using namespace std;

// json 序列化实例1
void func1()
{
    json js; //将js看成一个容器实现键值对
    // 
    js["msg_type"] = 2;
    js["from"] = "张三";
    js["to"] ="li si";
    js["msg"] = "hello nihao hhh!";

    std::cout<<js<<std::endl; //直接输出js就可以看到js的序列化效果了
    /*
    {"from":"张三","msg":"hello nihao hhh!","msg_type":2,"to":"li si"} // json肯定用<<重载
    和无序的关联容器感觉差不多，可以理解为用的是链式哈希表存储的，是无序的
    */
   //将序列化的数据转换成字符串进行发送
   string senBuf = js.dump();
   std::cout<<senBuf.c_str()<<std::endl; //通过网络传输传输的char*
   /*
   {"from":"张三","msg":"hello nihao hhh!","msg_type":2,"to":"li si"} // 字符串形式的，可以在网络上进行传输
   */
   
}

// 序列化实列2  可以封装更加复杂的数据(数组，json)
string func2()
{
    json js; //键值对里面的值可以整数，数组，字符串
    // 添加数组
    js["id"] = {1,2,3,4,5};
    // 添加key-value
    js["name"] = "zhang san";
    // 添加对象  //还可以想二维数组一样进行访问 msg下面也是个json类型
    js["msg"]["zhang san"] = "hello world";
    js["msg"]["liu shuo"] = "hello china";
    // 上面等同于下面这句一次性添加数组对象
    js["msg"] = {{"zhang san", "hello world"}, {"liu shuo", "hello china"}};
    //cout << js << endl;
    /*
    {"id":[1,2,3,4,5],"msg":{"liu shuo":"hello china","zhang san":"hello world"},"name":"zhang san"}
    */
   string s = js.dump();
   return s;

}
//json示例代码 可以实例容器
string func3()
{
    json js;
    // 直接序列化一个vector容器
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(5);

    js["list"] = vec; //将vector转成数组

    // 直接序列化一个map容器
    map<int, string> m;
    m.insert({1, "黄山"});
    m.insert({2, "华山"});
    m.insert({3, "泰山"});
    js["path"] = m; //map的键值对转成数组，path键对应的是大数组，里面套了三个map的小数组
   // cout<<js<<endl;
    /*
    {"list":[1,2,5],"path":[[1,"黄山"],[2,"华山"],[3,"泰山"]]}
    */
     //json数据对象 => 序列化 json字符串
   string senBuf = js.dump();
   // std::cout<<senBuf.c_str()<<std::endl;
    return senBuf;
}

int main()
{
    /*
    g++ -o testjson testjson.cpp -std=c++11 -g
    */
    
    // func1();
    // func3();
    string recvBuf = func2(); //接受到的字符流的string需要反序列化
    // 反序列化
    json jsbuf = json::parse(recvBuf); //将json字符串反序列化json对象
    // 输出出来还保留了数据类型
    //cout<<jsbuf["id"]<<jsbuf["msg"]<<endl; // [1,2,3,4,5]{"liu shuo":"hello china","zhang san":"hello world"}

    string recvBuf3 = func3();

    json jsbuf3 = json::parse(recvBuf3);
    vector<int> vec = jsbuf3["list"];
    for(int &v:vec)
    {
        cout<<v<<" ";
    }
    cout<<endl;

    return 0; 
}