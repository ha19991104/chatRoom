# cmake_minimum_required(VERSION 3.0)#要求cmake版本最低3.0
# project(muduo_server) #給 工程 一个名字
 
# # 配置编译选项 -o -g
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11") #给变量CMAKE_CXX_FLAGS的基础上多设置一个-g选项,也可以添加-O设置优化选项

#配置头文件搜索路径
#include_directories()

#配置库文件搜索路径
#link_directories()

#设置需要编译的源文件列表
set(SRC_LIST ./muduo_server.cpp) #给变量SRC_LIST赋值muduo_server.cpp

#如果cpp文件太多了,可以通过aux_source_directory来设置
# 把指定路径下的所有源文件名字放入变量SRC_LIST中  .表示当前目录下的路径
#aux_source_directory(. SRC_LIST)

#生成可执行文件server， 由SRC_LIST变量里面的所有源文件构成
#add_executable(server muduo_server.cpp) #通过muduo_server.cpp来生成可执行文件

#设置可执行文件最终存储的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) #当前工程根目录下的bin目录

#如果设置了SRC_LIST 就可以代替muduo_server.cpp
add_executable(server ${SRC_LIST})


#连接库   表示server可执行文件需要连接muduo_net muduo_base pthread这些库文件
target_link_libraries(server muduo_net muduo_base pthread)


# cmake .  是在当前目录执行CMakeLists.txt文件
# cmake .. 是在当前目录执行上一级目录中的CMakeLists.txt文件，且编译过程中的中间文件都产生在build目录中
# cmake .. 会将上级目录的可执行文件产生到当前目录中,所以需要set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)设置可执行文件的输出目录