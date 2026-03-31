cmake_minimum_required(VERSION 3.16)
project(Server)
set(CMAKE_CXX_STANDARD 17)
add_executable(Server
    main.cpp
    ClientHandler.cpp
    FleetDataManager.cpp
    FuelCalculator.cpp
    Logger.cpp
)
target_link_libraries(Server ws2_32)
target_compile_definitions(Server PRIVATE _WIN32_WINNT=0x0601)
