cmake_minimum_required(VERSION 3.12)
project(SDK_CalculixPlugin)

set(CMAKE_PREFIX_PATH
"/opt/Coreform-Cubit-2022.4/bin"
"C:/Program Files/Coreform Cubit 2022.4/bin")

# Get the packages needed to use the SDK
find_package(Cubit REQUIRED CONFIG)

set(SRC
    CalculiXPlugin.cpp
    CalculiXPlugin.hpp
    ccxVersionCommand.cpp
    ccxVersionCommand.hpp
    ccxExportCommand.cpp
    ccxExportCommand.hpp)

add_library(calculix_plugin MODULE ${SRC})
target_link_libraries(calculix_plugin cubiti cubit_util)
