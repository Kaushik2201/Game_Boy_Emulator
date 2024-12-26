cmake_minimum_required(VERSION 3.22.1)
project(Emulator)
add_library(mymath adder.cpp)
set_target_properties(mymath PROPERTIES PUBLIC_HEADER adder.h)
install(TARGETS mymath DESTINATION lib
        PUBLIC_HEADER DESTINATION include
        )

