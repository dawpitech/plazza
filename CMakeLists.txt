cmake_minimum_required(VERSION 3.10)

project(Plazza
        VERSION 1.0.0
        DESCRIPTION "Pasta pizza mama mia"
        LANGUAGES CXX
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g3 -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0")
endif()

file(GLOB_RECURSE sources "src/*.cpp")
add_executable(plazza ${sources})
target_include_directories(plazza PRIVATE src)
set_target_properties(plazza
        PROPERTIES PREFIX "${CMAKE_SOURCE_DIR}/"
)
