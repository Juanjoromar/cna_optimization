cmake_minimum_required(VERSION 3.10)

project(optimizing_entanglement_distribution VERSION 0.1.1 LANGUAGES CXX)

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(nlohmann_json REQUIRED)
#find_package(Boost REQUIRED)

include_directories(
    include
    ${nlohman_json_INCLUDE_DIRS}
)

add_executable(optimization
    src/genetic_algorithm.cpp
    src/network_model/entanglement_interface.cpp
    src/network_model/instruction.cpp
    src/network_model/instruction_table.cpp
    src/network_model/node.cpp
    src/network_model/network.cpp
    src/genetic_algorithm/definitions.cpp
)

target_link_libraries(optimization nlohmann_json::nlohmann_json)