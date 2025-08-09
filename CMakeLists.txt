cmake_minimum_required(VERSION 3.10)
project(MyCProgram C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

find_library(MICROHTTPD_LIBRARY microhttpd)
find_library(COLLECTC_LIBRARY collectc)
find_library(CJSON_LIBRARY cjson)

if(NOT MICROHTTPD_LIBRARY)
    message(FATAL_ERROR "libmicrohttpd not found")
endif()
if(NOT COLLECTC_LIBRARY)
    message(FATAL_ERROR "libcollectc not found")
endif()
if(NOT CJSON_LIBRARY)
    message(FATAL_ERROR "libcjson not found")
endif()

file(GLOB SOURCES "src/*.c")

add_executable(my-c-program ${SOURCES})

target_link_libraries(my-c-program PRIVATE
    ${MICROHTTPD_LIBRARY}
    ${COLLECTC_LIBRARY}
    ${CJSON_LIBRARY}
)

set_target_properties(my-c-program PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build"
)

add_custom_target(run
    COMMAND ${CMAKE_BINARY_DIR}/build/my-c-program
    DEPENDS my-c-program
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/build
    COMMENT "Running my-c-program"
)

add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/build
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMENT "Cleaning build artifacts"
)
