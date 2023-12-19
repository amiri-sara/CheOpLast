cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-g)

project(Aggregation LANGUAGES CXX)

include_directories(.)
include_directories(/home/mohammad/opencv/4.2.0/include/opencv4)

set(SRC_LIST main.cpp
             ReadConfigurations/configurate.cpp
             Cryptography/cipher.cpp
             Cryptography/cryptotools.cpp
             Cryptography/systemkeys.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} 
                      pthread 
                      crypto 
                      boost_thread 
                      boost_chrono 
                     )

target_link_libraries(${PROJECT_NAME} /usr/local/lib/libopencv_imgcodecs.so
                                      /usr/local/lib/libopencv_core.so)

