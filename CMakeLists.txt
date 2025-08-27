cmake_minimum_required(VERSION 3.5)

project(watch_me)

# autocomplete
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET (CMAKE_CXX_FLAGS "-Wall -Wextra -Wcast-qual -Wconversion-null -Wformat-security \
-Wmissing-declarations -Woverlength-strings -Wpointer-arith -Wundef \
-Wunused-local-typedefs -Wunused-result -Wvarargs -Wvla \
-Wwrite-strings -DNOMINMAX -fno-omit-frame-pointer -fPIC -g")

set(CMAKE_CXX_STANDARD 20)

set(SRC
    src/main.cpp
)

add_executable(${PROJECT_NAME} ${SRC})



include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.2.0
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3
)
FetchContent_MakeAvailable(spdlog)

find_package(OpenCL REQUIRED)
find_package(OpenCV REQUIRED)
target_link_libraries(${PROJECT_NAME} 
    PRIVATE OpenCL
    PRIVATE ${OpenCV_LIBS}
    PRIVATE fmt::fmt
    PRIVATE spdlog::spdlog
)

target_link_directories(${PROJECT_NAME} PRIVATE ${OpenCV_LINK_DIRECTORIES})
