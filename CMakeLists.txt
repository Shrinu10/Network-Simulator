cmake_minimum_required(VERSION 3.16)
project(RoutingSimulator VERSION 1.0 LANGUAGES CXX)

# ── C++17 Standard ──────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)       # No GNU extensions — ensures portability

# ── Executable ──────────────────────────────────────────────────
add_executable(routing_simulator
    src/main.cpp
    src/Config.cpp
    src/Packet.cpp
    src/Router.cpp
    src/RoutingTableGenerator.cpp
    src/Network.cpp
    src/Simulator.cpp
)

# ── Include Directories ────────────────────────────────────────
target_include_directories(routing_simulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ── Compiler Warnings (optional but recommended) ───────────────
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(routing_simulator PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(routing_simulator PRIVATE /W4)
endif()
