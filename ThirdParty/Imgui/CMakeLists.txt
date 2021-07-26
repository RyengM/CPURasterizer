cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME Imgui)
project(${PROJECT_NAME} LANGUAGES CXX)

include_directories("${CMAKE_SOURCE_DIR}/ThirdParty/OpenGL/include")
link_directories("${CMAKE_SOURCE_DIR}/ThirdParty/OpenGL/lib")
link_libraries(glfw3)

file(GLOB HEADER_FILES "*.h")
file(GLOB SOURCE_FILES "*.cpp" "*.c")

add_library(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/Lib"
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/Lib")