cmake_minimum_required(VERSION 3.0)
project(chatRoom)

#./bin/-可执行文件  
#./lib/-中间的库文件   这个项目并没有生成静态库和动态库，所以没设置lib目录
#./include/-头文件 
#./src/源文件  #
#./build/项目过程中，产生的临时文件
#./example/测试文件
#./thirdparty/第三方库文件 可以放json.hpp
#根目录下，放 CMakeLists.txt 再加一个 autobuild.sh（linuxshell脚本一键编译，实际执行的是cmake）

#配置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

#配置最终的可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#配置头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)  #如果路劲设置不对，或者路劲命出现错误会导致make编译找不到对应的头文件
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)
include_directories(${PROJECT_SOURCE_DIR}/thirdParty)
link_directories(/usr/lib64/mysql)  #libmysqlclient.os的连接路劲

# 加载子目录
add_subdirectory(src)

