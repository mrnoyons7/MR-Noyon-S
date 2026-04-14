cmake_minimum_required(VERSION 3.18.1)
project(goliath)

add_library(goliath SHARED native-lib.cpp)

find_library(log-lib log)
target_link_libraries(goliath ${log-lib})

target_compile_options(goliath PRIVATE
    -O3 -fvisibility=hidden -fdata-sections -ffunction-sections
)

set_target_properties(goliath PROPERTIES
    CXX_STANDARD 17
    ANDROID_ARM_NEON ON
    ANDROID_STL_TYPE c++_static
)
