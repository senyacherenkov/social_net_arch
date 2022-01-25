cmake_minimum_required(VERSION 3.5)

project(socialnetotus)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Poco REQUIRED Net)
find_package(Poco REQUIRED Util)
find_package(Poco REQUIRED Data)
find_package(MySQL REQUIRED)
find_package(Poco REQUIRED DataMySQL)

include_directories(${Poco_INCLUDE_DIRECTORIES})
add_executable(socialnetotus main.cpp)

target_link_libraries(socialnetotus ${Poco_LIBRARIES})


