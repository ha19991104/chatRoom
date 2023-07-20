cmake_minimum_required(VERSION 3.0)#要求cmake版本最低3.0
project(muduo_server) #給 工程 一个名字
 
# 配置编译选项 -o -g
#给变量CMAKE_CXX_FLAGS的基础上多设置一个-g选项,也可以添加-O设置优化选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11") 

#先找工程根目录下的cmakelist，再去指定目录下找cmakelist
#指定搜索的子目录，一般工程可能有很多目录
add_subdirectory(testfomuduo)