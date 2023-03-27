cmake_minimum_required(VERSION 3.0.0)
project(npl VERSION 0.1.0)
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
#SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${npl_SOURCE_DIR}/bin)
#SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


include_directories(lib/libnpl)

if (UNIX AND NOT APPLE)
    set (LINUX TRUE)
endif()
include(CTest)
enable_testing()

add_executable(main src/main.cpp)



set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
