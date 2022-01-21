cmake_minimum_required(VERSION 3.5)

project(social_net)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Poco REQUIRED Net)
find_package(Poco REQUIRED Util)
find_package(Poco REQUIRED Data)
find_package(MySQL REQUIRED)
find_package(Poco REQUIRED DataMySQL)

include_directories(${Poco_INCLUDE_DIRECTORIES})
add_executable(social_net main.cpp)

target_link_libraries(social_net ${Poco_LIBRARIES})


