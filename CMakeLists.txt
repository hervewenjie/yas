cmake_minimum_required(VERSION 3.5)
project(yas)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES src/main.c src/dbg.h src/util.h src/util.c src/epoll.h src/epoll.c src/http_request.h src/list.h src/http.h src/http_request.c src/error.h src/rio.h src/timer.h src/priority_queue.h src/priority_queue.c src/timer.c)
add_executable(yas ${SOURCE_FILES})