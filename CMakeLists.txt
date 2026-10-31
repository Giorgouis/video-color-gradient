
cmake_minimum_required(VERSION 3.20)
project(MyProject LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(FFmpeg REQUIRED IMPORTED_TARGET
    libavcodec
    libavformat
    libavutil
    libswscale
)

add_executable(my_app
    src/main.c
    src/file_handler.c
    src/frame_filter.c
)
target_include_directories(my_app PRIVATE include)
target_link_libraries(my_app PRIVATE
    PkgConfig::FFmpeg
)
