// #include "json.hpp"
// #include <iostream>
// #include <thread>
// #include <string>
// #include <vector>
// #include <chrono>
// #include <ctime>
// using namespace std;
// using json=nlohmann::json;

// #include <unistd.h>
// #include <sys/select.h>
// #include <sys/types.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <semaphore.h>
// #include <atomic>

// #include "group.hpp"
// #include "user.hpp"
// #include "public.hpp"

// // 记录当前系统登录的用户信息
// User g_currentUser;
// // 记录当前用户的好友列表信息
// vector<User> g_currentUserFriendList;
// // 记录当前登录用户的群组列表信息
// vector<Group> g_currentUserGroupList;

// // 控制主菜单页面程序
// bool isMainMenuRunning = false;

// // 用于读写线程之间的通信
// sem_t rwsem;
// // 记录登录状态
// atomic_bool g_isLoginSuccess{false};



// // 接收线程
// void readTaskHandler(int clientfd);
// // 获取系统时间(聊天信息需要添加时间消息)
// string getCurrentTime();
// //主聊天页面程序
// void mainMenu(int);
// // 显示当前登录成功用户的基本信息
// void showCurrentUserData();

// int main(int argc, char **argv)
// {
//     if (argc < 3)
//     {
//         cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << endl;
//         exit(-1);
//     }

//     // 解析通过命令行参数传递的ip和port
//     char *ip = argv[1];
//     uint16_t port = atoi(argv[2]);

//     // 创建client端的socket
//     int clientfd = socket(AF_INET, SOCK_STREAM, 0);
//     if (-1 == clientfd)
//     {
//         cerr << "socket create error" << endl;
//         exit(-1);
//     }

//     // 填写client需要连接的server信息ip+port
//     sockaddr_in server;
//     memset(&server, 0, sizeof(sockaddr_in));

//     server.sin_family = AF_INET;
//     server.sin_port = htons(port);
//     server.sin_addr.s_addr = inet_addr(ip);

//     // client和server进行连接
//     if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
//     {
//         cerr << "connect server error" << endl;
//         close(clientfd);
//         exit(-1);
//     }

//     // 初始化读写线程通信用的信号量
//     sem_init(&rwsem, 0, 0); //第二个参数0表示线程共享，第三个参数是初始化的资源数量
//     /*
//         当资源数是1的时候，多个线程调用sem_wait，只有一个线程会被唤醒
//     */

//     // 连接服务器成功，启动接收子线程
//     std::thread readTask(readTaskHandler, clientfd); // pthread_create
//     readTask.detach();                               // pthread_detach

//     // main线程用于接收用户输入，负责发送数据
//     for (;;)
//     {
//         // 显示首页面菜单 登录、注册、退出
//         cout << "========================" << endl;
//         cout << "1. login" << endl;
//         cout << "2. register" << endl;
//         cout << "3. quit" << endl;
//         cout << "========================" << endl;
//         cout << "choice:";
//         int choice = 0;
//         cin >> choice;
//         cin.get(); // 读掉缓冲区残留的回车

//         switch (choice)
//         {
//         case 1: // login业务
//         {
//             int id = 0;
//             char pwd[50] = {0};
//             cout << "userid:";
//             cin >> id;
//             cin.get(); // 读掉缓冲区残留的回车
//             cout << "userpassword:";
//             cin.getline(pwd, 50);

//             json js;
//             js["msgid"] = LOGIN_MSG;
//             js["id"] = id;
//             js["password"] = pwd;
//             string request = js.dump();

//             g_isLoginSuccess = false;

//             int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
//             if (len == -1)
//             {
//                 cerr << "send login msg error:" << request << endl;
//             }

//             sem_wait(&rwsem); // 等待信号量，由子线程处理完登录的响应消息后，通知这里
//             // 资源数为0，sem_wait会阻塞等待资源
//             // 成功返回0，失败返回-1，errno是EINTR是中断信号。
                
//             if (g_isLoginSuccess) 
//             {
//                 // 进入聊天主菜单页面
//                 isMainMenuRunning = true;
//                 mainMenu(clientfd);
//             }
//         }
//         break;
//         case 2: // register业务
//         {
//             char name[50] = {0};
//             char pwd[50] = {0};
//             cout << "username:";
//             cin.getline(name, 50);
//             cout << "userpassword:";
//             cin.getline(pwd, 50);

//             json js;
//             js["msgid"] = REG_MSG;
//             js["name"] = name;
//             js["password"] = pwd;
//             string request = js.dump();

//             int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
//             if (len == -1)
//             {
//                 cerr << "send reg msg error:" << request << endl;
//             }
            
//             sem_wait(&rwsem); // 等待信号量，子线程处理完注册消息会通知
//         }
//         break;
//         case 3: // quit业务
//             close(clientfd);
//             sem_destroy(&rwsem); //销毁信号量
//             exit(0);
//         default:
//             cerr << "invalid input!" << endl;
//             break;
//         }
//     }

//     return 0;
// }


// // 处理注册的响应逻辑
// void doRegResponse(json &responsejs)
// {
//     if (0 != responsejs["errno"].get<int>()) // 注册失败
//     {
//         cerr << "name is already exist, register error!" << endl;
//     }
//     else // 注册成功
//     {
//         cout << "name register success, userid is " << responsejs["id"]
//                 << ", do not forget it!" << endl;
//     }
// }

// // 处理登录的响应逻辑
// void doLoginResponse(json &responsejs)
// {
//     if (0 != responsejs["errno"].get<int>()) // 登录失败
//     {
//         cerr << responsejs["errmsg"] << endl;
//         g_isLoginSuccess = false;
//     }
//     else // 登录成功
//     {
//         // 记录当前用户的id和name
//         g_currentUser.setId(responsejs["id"].get<int>());
//         g_currentUser.setName(responsejs["name"]);

//         // 记录当前用户的好友列表信息
//         if (responsejs.contains("friends"))
//         {
//             // 初始化
//             g_currentUserFriendList.clear();

//             vector<string> vec = responsejs["friends"];
//             for (string &str : vec)
//             {
//                 json js = json::parse(str);
//                 User user;
//                 user.setId(js["id"].get<int>());
//                 user.setName(js["name"]);
//                 user.setState(js["state"]);
//                 g_currentUserFriendList.push_back(user);
//             }
//         }

//         // 记录当前用户的群组列表信息
//         if (responsejs.contains("groups"))
//         {
//             // 初始化
//             g_currentUserGroupList.clear();

//             vector<string> vec1 = responsejs["groups"];
//             for (string &groupstr : vec1)
//             {
//                 json grpjs = json::parse(groupstr);
//                 Group group;
//                 group.setId(grpjs["id"].get<int>());
//                 group.setNmae(grpjs["groupname"]);
//                 group.setDesc(grpjs["groupdesc"]);

//                 vector<string> vec2 = grpjs["users"];
//                 for (string &userstr : vec2)
//                 {
//                     GroupUser user;
//                     json js = json::parse(userstr);
//                     user.setId(js["id"].get<int>());
//                     user.setName(js["name"]);
//                     user.setState(js["state"]);
//                     user.setRole(js["role"]);
//                     group.getUsers().push_back(user);
//                 }

//                 g_currentUserGroupList.push_back(group);
//             }
//         }

//         // 显示登录用户的基本信息
//         showCurrentUserData();

//         // 显示当前用户的离线消息  个人聊天信息或者群组消息
//         if (responsejs.contains("offlinemsg"))
//         {
//             vector<string> vec = responsejs["offlinemsg"];
//             for (string &str : vec)
//             {
//                 json js = json::parse(str);
//                 // time + [id] + name + " said: " + xxx
//                 if (ONE_CHAT_MSG == js["msgid"].get<int>())
//                 {
//                     cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
//                             << " said: " << js["msg"].get<string>() << endl;
//                 }
//                 else
//                 {
//                     cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
//                             << " said: " << js["msg"].get<string>() << endl;
//                 }
//             }
//         }

//         g_isLoginSuccess = true;
//     }
// }

// // 子线程 - 接收线程
// void readTaskHandler(int clientfd)
// {
//     for (;;) //子线程一直循环
//     {
//         char buffer[1024] = {0};
//         int len = recv(clientfd, buffer, 1024, 0);  // 阻塞了
//         if (-1 == len || 0 == len) //0是对端关闭，-1是出错了
//         {
//             close(clientfd);
//             exit(-1);
//         }

//         // 接收ChatServer转发的数据，反序列化生成json数据对象
//         json js = json::parse(buffer);
//         int msgtype = js["msgid"].get<int>();
//         if (ONE_CHAT_MSG == msgtype)
//         {
//             cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
//                  << " said: " << js["msg"].get<string>() << endl;
//             continue;
//         }

//         if (GROUP_CHAT_MSG == msgtype)
//         {
//             cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
//                  << " said: " << js["msg"].get<string>() << endl;
//             continue;
//         }

//         if (LOGIN_MSG_ACK == msgtype)
//         {
//             doLoginResponse(js); // 处理登录响应的业务逻辑
//             sem_post(&rwsem);    // 通知主线程，登录结果处理完成
//             continue;
//         }

//         if (REG_MSG_ACK == msgtype)
//         {
//             doRegResponse(js);
//             sem_post(&rwsem);    // 通知主线程，注册结果处理完成
//             continue;
//         }
//     }
// }
// //主聊天页面程序
// void mainMenu(int clientfd)
// {

// }
// // 显示当前登录成功用户的基本信息
// void showCurrentUserData()
// {
//     cout << "======================login user======================" << endl;
//     cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;
//     cout << "----------------------friend list---------------------" << endl;
//     if (!g_currentUserFriendList.empty())
//     {
//         for (User &user : g_currentUserFriendList)
//         {
//             cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
//         }
//     }
//     cout << "----------------------group list----------------------" << endl;
//     if (!g_currentUserGroupList.empty())
//     {
//         for (Group &group : g_currentUserGroupList)
//         {
//             cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
//             for (GroupUser &user : group.getUsers())
//             {
//                 cout << user.getId() << " " << user.getName() << " " << user.getState()
//                      << " " << user.getRole() << endl;
//             }
//         }
//     }
//     cout << "======================================================" << endl;
// }