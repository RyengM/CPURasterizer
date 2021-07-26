cmake_minimum_required(VERSION 3.15)

set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_CXX_STANDARD 17)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PROJECT_NAME SelfLearningTask)
project(${PROJECT_NAME} LANGUAGES CXX)

add_subdirectory(Source/Renderer)
add_subdirectory(Source/Editor)
add_subdirectory(Source/WinEditor)
add_subdirectory(ThirdParty/Imgui)