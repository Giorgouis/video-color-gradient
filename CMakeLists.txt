
cmake_minimum_required(VERSION 3.20)
project(MyProject LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(FFmpeg REQUIRED IMPORTED_TARGET
    libavcodec
    libavformat
    libavutil
)

add_executable(my_app
    src/main.c
)

target_link_libraries(my_app PRIVATE
    PkgConfig::FFmpeg
)
