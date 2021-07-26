cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXE_LINKER_FLAGS "${CMKAE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
set(CMAKE_RC_COMPILE_OBJECT "${CMAKE_RC_COMPILER} -O coff -I${CMAKE_CURRENT_SOURCE_DIR} <SOURCE> <OBJECT>")

set(PROJECT_NAME WinEditor)
project(${PROJECT_NAME} LANGUAGES CXX)
enable_language(RC)
set(SOURCES WinEditor.rc ${SOURCES})

include_directories("${CMAKE_SOURCE_DIR}/Include")
link_directories("${CMAKE_SOURCE_DIR}/Lib")
link_libraries(Renderer)

file(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/Include/WinEditor/*.h")
file(GLOB SOURCE_FILES "*.c" "*.cpp")
file(GLOB RESOURCE_FILES "Resource/*.ico" "Resource/*.rc")

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/Bin)

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin/$(Configuration)")