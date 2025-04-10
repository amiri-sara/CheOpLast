cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pg")

project(checkOperator LANGUAGES CXX)
message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER}")

OPTION(BUILDUBUNTU22 "Build in Ubuntu22" ON)
OPTION(BUILDUBUNTU20 "Build in Ubuntu20" OFF)

OPTION(CLIENTSERVICE "Run client service" ON)
OPTION(WEBSERVICE "Run Web service" OFF)
OPTION(KAFKASERVICE "Run Kafka service" OFF)
OPTION(INSERTDATABASE "Insert the data into the database" ON)
OPTION(FAILEDDATABASE "failed database" ON)
OPTION(STOREIMAGE "Store Image" ON)
OPTION(KAFKAOUTPUT "Add the output data to the Kafka" OFF)
OPTION(VALUEVALIDATION "Validation value of input" OFF)

find_package(Boost COMPONENTS system filesystem  REQUIRED)

set(WITH_GPU OFF)
if(WITH_GPU)
    add_definitions(-DWITH_GPU)
    set(CUDA_DIR /usr/local/cuda-11.6)
    include_directories(${CUDA_DIR}/include)
    link_directories(${CUDA_DIR}/lib64)
    set(TRT_DIR /usr/local/TensorRT-8.4.0.6)
    include_directories(${TRT_DIR}/include)
    link_directories(${TRT_DIR}/lib)
endif()

if(BUILDUBUNTU22)
    add_definitions(-DCREATEMONGOINSTANCE)
endif()

if(BUILDUBUNTU20)
    add_definitions(-DCREATEMONGOINSTANCE)
endif()

if(CLIENTSERVICE)
    add_definitions(-DCLIENTSERVICE)
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

file(GLOB_RECURSE HEADER_FILES "./*.h")
set(HEADER_DIRS "")
foreach(header ${HEADER_FILES})
    get_filename_component(HEADER_DIR ${header} PATH)
    list(APPEND HEADER_DIRS ${HEADER_DIR})
endforeach()
list(REMOVE_DUPLICATES HEADER_DIRS)
include_directories(${HEADER_DIRS})

set(inc_dir /usr/local/include)
if(BUILDUBUNTU20)
    include_directories(/etc/programs/opencv-_pervios/4.2.0/include/opencv4)
    set(OPENCVLIB_dir /usr/local/lib)
elseif(BUILDUBUNTU22)
    include_directories(${inc_dir}/opencv/4.7.0/opencv4)
    set(OPENCVLIB_dir /usr/local/lib/opencv/4.7.0)
endif()

include_directories(.
                    /home/amiri/programs/rapidjson/include
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
             checkop/chop.cpp #TODO must remove make .so
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

if(CLIENTSERVICE)
    set(SRC_LIST    ${SRC_LIST}
                    Service/Rahdari/rahdariService.cpp)
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

set(onnxlib_dir  /usr/local/lib/onnx/1.17.1)

set(INFERENCELIB_dir /usr/local/lib/kmodules)
set(INFERENCELIB     ${INFERENCELIB_dir}/libInference.so)

# set(CHOPLIB_dir /usr/local/lib/kmodules)#TODO
# set(CHOPLIB     ${CHOPLIB_dir}/libchop.so)

set(CLASSIFIERLIB_dir /usr/local/lib/kmodules)
set(CLASSIFIERLIB     ${CLASSIFIERLIB_dir}/libclassifier.so)

target_link_libraries(${PROJECT_NAME} 
                      curl
                      pthread
                      crypto 
                      boost_thread 
                      boost_chrono
                      boost_filesystem 
                      ${DEEP_CORES}
                      ${OPENCVLIB}
                      ${INFERENCELIB}
                      ${CHOPLIB}
                      ${CLASSIFIERLIB}
                      ${onnxlib_dir}/libonnxruntime.so
                      /usr/lib/x86_64-linux-gnu/libssl.so.3 
                      /usr/lib/x86_64-linux-gnu/libcrypto.so.3
                      /usr/lib/x86_64-linux-gnu/libfreeimage.so)

                      
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
