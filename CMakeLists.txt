cmake_minimum_required(VERSION 3.21)
project(BluetoothExt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /UMBCS  /permissive /MP /GS /GL /W4 /Zc:wchar_t  /guard:cf /Gm- /O2 /sdl-  /Zc:inline /fp:precise /D " _WINDLL" /D "_UNICODE" /D "UNICODE"  /GF /GT /WX- /Zc:forScope  /Gd /Oi /MT /std:c++20   /EHsc /nologo /Ot")


add_library(BluetoothExt SHARED
        log.c
        log.h
        utils.cpp
        utils.h
        bluetooth_ext.cpp
        bluetooth_ext.h
)
add_executable(main main.cpp bluetooth_ext.cpp utils.cpp log.c)

target_link_libraries(main PRIVATE ws2_32)
target_link_libraries(BluetoothExt PRIVATE ws2_32)

