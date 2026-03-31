cmake_minimum_required(VERSION 3.16)
project(Client)
set(CMAKE_CXX_STANDARD 17)
add_executable(Client
    main.cpp
    TelemetryFileReader.cpp
    PacketBuilder.cpp
    TcpTransmitter.cpp
)
target_link_libraries(Client ws2_32)
target_compile_definitions(Client PRIVATE _WIN32_WINNT=0x0601)
