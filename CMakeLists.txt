cmake_minimum_required(VERSION 3.28)
project(sysops)

set(CMAKE_CXX_STANDARD 14)

include_directories(cw07)

add_executable(sysops
        cw07/client.c
        cw07/Makefile
        cw07/queue_spec.h
        cw07/server.c)

add_executable(cw08
        cw08/printers_server.c
        cw08/users_client.c
        cw08/Makefile
        cw08/shared_memory_spec.h)
