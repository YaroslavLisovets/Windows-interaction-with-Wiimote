cmake_minimum_required(VERSION 3.21)
project(WiimoteHandler)
set(CMAKE_CXX_STANDARD 23)


add_executable(WiimoteHandler main.cpp)
add_library(WiimoteHid STATIC WiimoteHid.cpp)

target_link_libraries(WiimoteHandler PRIVATE WiimoteHid)
target_link_libraries(WiimoteHid wsock32 ws2_32 bluetoothapis)
target_link_libraries(WiimoteHid "C:/Program\ Files\ (x86)/Windows\ Kits/10/Lib/10.0.19041.0/um/x64/hid.lib")
target_link_libraries(WiimoteHid "C:/Program\ Files\ (x86)/Windows\ Kits/10/Lib/10.0.19041.0/um/x64/SetupAPI.Lib")