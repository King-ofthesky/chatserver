#项目名字
project(serverV1)
#项目最小版本
cmake_minimum_required(VERSION 3.25)
#设置编译器编译选项
set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_FLAGS  ${CMAKE_CXX_FLAGS} -g )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ")
#设置编译结果输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#设置代码的文件夹
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/Chatserver)
include_directories(${CMAKE_SOURCE_DIR}/include/Server)


#自动搜索编译
aux_source_directory(${CMAKE_SOURCE_DIR}/src/Server SRC_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/src/Chatserver SRC_LIST)
#aux_source_directory(${CMAKE_SOURCE_DIR}/src/Proto SRC_LIST)


#进行编译 
add_executable(serverV1 ${SRC_LIST})
#添加连接库
target_link_libraries( serverV1 mysqlclient) 
