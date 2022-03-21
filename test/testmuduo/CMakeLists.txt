cmake_minimum_required(VERSION 2.8)
project(main)

#配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

#添加头文件搜索路径
#include_directories()
#link_directories()       配置库文件搜索路径

#设置需要编译的源文件列表
set(SRC_LIST  ./ muduo_server.cpp)

#add_executable(server  muduo_server.cpp)   #通过muduo ~~生成可执行文件server
#设置最终可执行文件的存储位置  ，存储在该项目目录下的bin目录下
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#由SRC_LIST变量所定义的源文件编译生成可执行文件server
add_executable(server ${SRC_LIST})

#可执行文件需要链接的库
target_link_libraries(server muduo_net muduo_base pthread)