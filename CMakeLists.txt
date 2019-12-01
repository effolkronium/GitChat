cmake_minimum_required(VERSION 3.5)

project(GitChat LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(GitChat
  main.cpp
  GitChatConfig.cpp
  GitChatConfig.h
  GitChatConfig.ui
  ListViewDelegate.cpp
  ListViewDelegate.h
  GitChat.cpp
  GitChat.h
  GitChat.ui
  Git.cpp
  Git.h
  thread_safe_queue.hpp
)

target_link_libraries(GitChat PRIVATE Qt5::Widgets Threads::Threads)
