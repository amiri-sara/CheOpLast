cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-g)

project(Aggregation LANGUAGES CXX)

include_directories(.)
include_directories(/home/mohammad/opencv/4.2.0/include/opencv4)
include_directories(.
                    /usr/local/include/mongocxx/v_noabi
                    /usr/local/include/bsoncxx/v_noabi
                    /usr/local/include/bsoncxx/v_noabi/bsoncxx/third_party/mnmlstc
                    /usr/local/include/libmongoc-1.0
                    /usr/local/include/libbson-1.0)

if (DEFINED ENV{LIBRDKAFKA_INCLUDE_DIR})
    set(LIBRDKAFKA_INCLUDE_DIR $ENV{LIBRDKAFKA_INCLUDE_DIR})
else ()
    find_file(LIBRDKAFKA_HEADER
            NAMES rdkafka.h
            HINTS /usr/include/librdkafka /usr/local/include/librdkafka /opt/homebrew/include/librdkafka)
                
    cmake_path(GET LIBRDKAFKA_HEADER PARENT_PATH LIBRDKAFKA_INCLUDE_DIR)
    cmake_path(GET LIBRDKAFKA_INCLUDE_DIR PARENT_PATH LIBRDKAFKA_INCLUDE_DIR)
endif ()
                
if (DEFINED ENV{LIBRDKAFKA_LIBRARY_DIR})
    set(LIBRDKAFKA_LIBRARY_DIR $ENV{LIBRDKAFKA_LIBRARY_DIR})
else ()
    find_library(LIBRDKAFKA_LIB
                NAMES rdkafka
                HINTS /usr/lib /usr/local/lib /opt/homebrew/lib)
                
    cmake_path(GET LIBRDKAFKA_LIB PARENT_PATH LIBRDKAFKA_LIBRARY_DIR)
endif ()
                
if (EXISTS "${LIBRDKAFKA_INCLUDE_DIR}/librdkafka/rdkafka.h")
    message(STATUS "librdkafka include directory: ${LIBRDKAFKA_INCLUDE_DIR}")
else ()
    message(FATAL_ERROR "Could not find headers for librdkafka!")
endif ()
                
if (EXISTS "${LIBRDKAFKA_LIBRARY_DIR}/librdkafka.a" OR EXISTS "${LIBRDKAFKA_LIBRARY_DIR}/librdkafka.so" OR EXISTS "${LIBRDKAFKA_LIBRARY_DIR}/rdkafka.lib" )
    message(STATUS "librdkafka library directory: ${LIBRDKAFKA_LIBRARY_DIR}")
else ()
    message(FATAL_ERROR "Could not find library for librdkafka!")
endif ()

FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0)

FILE(GLOB SRC src/*.c)

find_package(PkgConfig REQUIRED)
pkg_check_modules(rdkafka REQUIRED IMPORTED_TARGET rdkafka)
                                      
find_package(PkgConfig REQUIRED)
pkg_check_modules(rdkafka++ REQUIRED IMPORTED_TARGET rdkafka++)

include_directories(${GLIB_INCLUDE_DIRS})
LINK_DIRECTORIES(${GLIB_LIBRARY_DIRS})

set(SRC_LIST main.cpp
             ReadConfigurations/configurate.cpp
             Cryptography/cipher.cpp
             Cryptography/cryptotools.cpp
             Cryptography/systemkeys.cpp
             Database/MongoDB.cpp
             Service/WebService/webservice.cpp
             Service/KafkaService/kafkaservice.cpp
             Kafka/KafkaProsumer.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} 
                      pthread 
                      crypto 
                      boost_thread 
                      boost_chrono 
                      ${GLIB_LIBRARIES} 
                      rdkafka++
                     )

target_link_libraries(${PROJECT_NAME} /usr/local/lib/libopencv_imgcodecs.so
                                      /usr/local/lib/libopencv_core.so
                                      /usr/local/lib/libmongocxx.so
                                      /usr/local/lib/libbsoncxx.so)

