cmake_minimum_required(VERSION 3.10.0)
project(ee)

set(SRCS 
    eeprom.c
    )

add_executable(ee main.c ${SRCS})