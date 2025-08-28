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
    src/server.cpp
)

add_executable(${PROJECT_NAME} ${SRC})
add_executable(client src/client.cpp)


include(FindPkgConfig)
pkg_check_modules(GSTREAMER gstreamer-1.0>=1.6 REQUIRED)
include_directories(/usr/include/glib-2.0 /usr/lib/x86_64-linux-gnu/glib-2.0/include)
include_directories(/usr/include/gstreamer-1.0/)
link_directories(${GSTREAMER_LIBRARY_DIRS})
include(CMakePrintHelpers)

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
    PRIVATE ${GSTREAMER_LIBRARIES}
)

target_link_directories(${PROJECT_NAME} PRIVATE ${OpenCV_LINK_DIRECTORIES} ${GSTREAMER_LIBRARIES})

target_link_libraries(client
    PRIVATE OpenCL
    PRIVATE ${OpenCV_LIBS}
    PRIVATE fmt::fmt
    PRIVATE spdlog::spdlog
    PRIVATE ${GSTREAMER_LIBRARIES}
)

target_link_directories(client PRIVATE ${OpenCV_LINK_DIRECTORIES} ${GSTREAMER_LIBRARIES})
