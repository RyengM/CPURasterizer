cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME Editor)
project(${PROJECT_NAME} LANGUAGES CXX)

include_directories("${CMAKE_SOURCE_DIR}/ThirdParty/Imgui" "${CMAKE_SOURCE_DIR}/ThirdParty/OpenGL/include" "${CMAKE_SOURCE_DIR}/Include")
link_directories("${CMAKE_SOURCE_DIR}/Lib" "${CMAKE_SOURCE_DIR}/ThirdParty/OpenGL/lib")
link_libraries(Imgui glfw3 Renderer)

file(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/Include/Editor/*.h")
file(GLOB SOURCE_FILES "*.c" "*.cpp")

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/Bin)

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin/$(Configuration)")