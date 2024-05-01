cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-g)

project(Aggregation LANGUAGES CXX)

OPTION(BUILDUBUNTU22 "Build in Ubuntu22" ON)
OPTION(BUILDUBUNTU20 "Build in Ubuntu20" OFF)

OPTION(WEBSERVICE "Run Web service" ON)
OPTION(KAFKASERVICE "Run Kafka service" ON)
OPTION(INSERTDATABASE "Insert the data into the database" ON)
OPTION(FAILEDDATABASE "failed database" OFF)
OPTION(STOREIMAGE "Store Image" ON)
OPTION(KAFKAOUTPUT "Add the output data to the Kafka" ON)
OPTION(VALUEVALIDATION "Validation value of input" ON)

if(BUILDUBUNTU22)
    add_definitions(-DCREATEMONGOINSTANCE)
endif()

if(WEBSERVICE)
    add_definitions(-DWEBSERVICE)
endif()

if(KAFKASERVICE)
    add_definitions(-DKAFKASERVICE)
endif()

if(INSERTDATABASE)
    add_definitions(-DINSERTDATABASE)
endif()

if(FAILEDDATABASE)
    add_definitions(-DFAILEDDATABASE)
endif()

if(STOREIMAGE)
    add_definitions(-DSTOREIMAGE)
endif()

if(KAFKAOUTPUT)
    add_definitions(-DKAFKAOUTPUT)
endif()

if(VALUEVALIDATION)
    add_definitions(-DVALUEVALIDATION)
endif()

include_directories(.)

set(inc_dir /usr/local/include)
if(BUILDUBUNTU20)
    include_directories(/home/mohammad/opencv/4.2.0/include/opencv4)
    set(OPENCVLIB_dir /usr/local/lib)
elseif(BUILDUBUNTU22)
    include_directories(${inc_dir}/opencv/4.7.0/opencv4)
    set(OPENCVLIB_dir /usr/local/lib/opencv/4.7.0)
endif()

include_directories(.
                    /usr/local/include/mongocxx/v_noabi
                    /usr/local/include/bsoncxx/v_noabi
                    /usr/local/include/bsoncxx/v_noabi/bsoncxx/third_party/mnmlstc
                    /usr/local/include/libmongoc-1.0
                    /usr/local/include/libbson-1.0
                    ${inc_dir}/onnx/1.17.1)

set(OPENCVLIB       ${OPENCVLIB_dir}/libopencv_imgcodecs.so
                    ${OPENCVLIB_dir}/libopencv_core.so
                    ${OPENCVLIB_dir}/libopencv_videoio.so
                    ${OPENCVLIB_dir}/libopencv_imgproc.so
                    ${OPENCVLIB_dir}/libopencv_highgui.so
                    ${OPENCVLIB_dir}/libopencv_dnn.so
                    ${OPENCVLIB_dir}/libopencv_freetype.so
                    ${OPENCVLIB_dir}/libopencv_dnn.so)

if(KAFKASERVICE OR KAFKAOUTPUT)
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
endif()

set(SRC_LIST main.cpp
             ReadConfigurations/configurate.cpp
             Cryptography/cipher.cpp
             Cryptography/cryptotools.cpp
             Cryptography/systemkeys.cpp
             Database/MongoDB.cpp
             Validator/validator.cpp
             Time/timetools.cpp
             Service/Service.cpp)

if(KAFKASERVICE OR KAFKAOUTPUT)
    set(SRC_LIST    ${SRC_LIST}
                    Service/KafkaService/kafkaservice.cpp
                    Kafka/KafkaProsumer.cpp)
endif()
if(WEBSERVICE)
    set(SRC_LIST    ${SRC_LIST}
                    Service/WebService/webservice.cpp)
endif()

if(STOREIMAGE)
    set(SRC_LIST    ${SRC_LIST}
                    StoreImage/storeimage.cpp)
endif()

if(INSERTDATABASE OR KAFKAOUTPUT)
    set(SRC_LIST    ${SRC_LIST}
                    SaveData/savedata.cpp)
endif()

add_executable(${PROJECT_NAME} ${SRC_LIST})

set(onnxlib_dir /usr/local/lib/onnx/1.17.1)

set(IDLMLIB_dir /usr/local/lib/kmodules)
set(IDLMLIB     ${IDLMLIB_dir}/libIDLM.so)

set(CHOPLIB_dir /usr/local/lib/kmodules)
set(CHOPLIB     ${CHOPLIB_dir}/libCheckOP.so)

target_link_libraries(${PROJECT_NAME} 
                      curl
                      pthread 
                      crypto 
                      boost_thread 
                      boost_chrono
                      boost_filesystem 
                      ${OPENCVLIB}
                      ${IDLMLIB}
                      ${CHOPLIB}
                      ${onnxlib_dir}/libonnxruntime.so)

if(KAFKASERVICE OR KAFKAOUTPUT)
    target_link_libraries(${PROJECT_NAME} 
                          ${GLIB_LIBRARIES} 
                          rdkafka++)
endif()

if(BUILDUBUNTU20)
    target_link_libraries(${PROJECT_NAME} /usr/local/lib/libmongocxx.so
                                          /usr/local/lib/libbsoncxx.so)
elseif(BUILDUBUNTU22)
    target_link_libraries(${PROJECT_NAME} /usr/local/lib/mongocxx/libmongocxx.so
                                          /usr/local/lib/mongocxx/libbsoncxx.so)
endif()

if(BUILDUBUNTU20)
    target_link_libraries(${PROJECT_NAME} /usr/local/lib/libbannerapi.so)
elseif(BUILDUBUNTU22)
    target_link_libraries(${PROJECT_NAME} /usr/local/lib/banner/libbannerapi.so)
endif()