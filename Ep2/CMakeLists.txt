cmake_minimum_required(VERSION 3.13)
project(murlyMath)
add_library(murlyMath SomeLib/cpp/adder.cpp)
set_target_properties(murlyMath PROPERTIES PUBLIC_HEADER cpp/adder.h)
# install(TARGETS murlyMath DESTINATION lib
#     PUBLIC_HEADER DESTINATION include
# )

# cmake_minimum_required(VERSION 3.13)
# project(murlyMath)

# add_library(murlyMath STATIC ${CMAKE_SOURCE_DIR}/SomeLib/cpp/adder.cpp)

# set_target_properties(murlyMath PROPERTIES PUBLIC_HEADER ${CMAKE_SOURCE_DIR}/SomeLib/cpp/adder.h)

# install(TARGETS murlyMath 
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib             
#     PUBLIC_HEADER DESTINATION include/murlyMath 
# )
