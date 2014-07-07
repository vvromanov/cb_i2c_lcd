cmake_minimum_required(VERSION 2.8.8)
set(TARGET i2c_lcd)
project(${TARGET})

# main
set(${TARGET}_SRCS
    i2c_lcd.c
)

add_definitions(-DPROG_NAME=\"${TARGET}\")

# build!
include_directories(${UTILS_INCS})

add_executable(${TARGET} ${${TARGET}_SRCS})

