cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME Renderer)
project(${PROJECT_NAME} LANGUAGES CXX)

include_directories("${CMAKE_SOURCE_DIR}/Include" "${CMAKE_SOURCE_DIR}/ThirdParty/stb")

file(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/Include/Renderer/*.h" "${CMAKE_SOURCE_DIR}/Include/Renderer/Detail/*.inl")
file(GLOB SOURCE_FILES "${CMAKE_SOURCE_DIR}/Source/Renderer/*.cpp")

add_library(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/Lib"
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/Lib")