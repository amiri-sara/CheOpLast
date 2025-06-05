cmake_minimum_required(VERSION 3.5)
project(checkOperator LANGUAGES CXX) # Default project name, overridden later

# Compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pg")

# Project name options
option(LOADDETECTOR "Enable load detector mode" OFF)
option(CHECKOPERATOR "Enable check operator mode" ON)

if(LOADDETECTOR AND CHECKOPERATOR)
    message(FATAL_ERROR "Cannot enable both LOADDETECTOR and CHECKOPERATOR")
elseif(LOADDETECTOR)
    set(PROJECT_NAME LoadDetection)
elseif(CHECKOPERATOR)
    set(PROJECT_NAME checkOperator)
else()
    set(PROJECT_NAME checkOperator)
endif()
project(${PROJECT_NAME} LANGUAGES CXX)
message(STATUS "Project name: ${PROJECT_NAME}")

# Compiler version check
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    message(FATAL_ERROR "GCC version too old. Need at least GCC 9.")
endif()
message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER} (version ${CMAKE_CXX_COMPILER_VERSION})")

# Build options
option(BUILDUBUNTU22 "Build for Ubuntu 22.04" ON)
option(BUILDUBUNTU20 "Build for Ubuntu 20.04" OFF)
option(CLIENTSERVICE "Run client service" ON)
option(WEBSERVICE "Run web service" OFF)
option(KAFKASERVICE "Run Kafka service" OFF)
option(INSERTDATABASE "Insert data into database" ON)
option(FAILEDDATABASE "Handle failed database" OFF)
option(STOREIMAGE "Store images" ON)
option(KAFKAOUTPUT "Output data to Kafka" OFF)
option(VALUEVALIDATION "Validate input values" OFF)
option(WITH_GPU "Enable GPU support" OFF)

# Validate Ubuntu options
if(BUILDUBUNTU20 AND BUILDUBUNTU22)
    message(FATAL_ERROR "Cannot enable both BUILDUBUNTU20 and BUILDUBUNTU22")
elseif(NOT BUILDUBUNTU20 AND NOT BUILDUBUNTU22)
    message(FATAL_ERROR "Must enable either BUILDUBUNTU20 or BUILDUBUNTU22")
endif()

# Base directories
set(INC_DIR /usr/local/include)
set(LIB_DIR /usr/local/lib)
set(INFERENCELIB_DIR ${LIB_DIR}/kmodules)

# Dependencies
find_package(Boost 1.71 REQUIRED COMPONENTS system filesystem thread chrono)
find_package(PkgConfig QUIET)

# OpenCV configuration
if(BUILDUBUNTU20)
    set(OPENCV_INCLUDE_DIR /etc/programs/opencv-_pervios/4.2.0/include/opencv4)
    set(OPENCV_LIB_DIR ${LIB_DIR})
elseif(BUILDUBUNTU22)
    set(OPENCV_INCLUDE_DIR ${INC_DIR}/opencv/4.7.0/opencv4)
    set(OPENCV_LIB_DIR ${LIB_DIR}/opencv/4.7.0)
endif()

# Validate OpenCV paths
foreach(path "${OPENCV_INCLUDE_DIR}/opencv2/core.hpp" "${OPENCV_LIB_DIR}/libopencv_core.so")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "OpenCV path not found: ${path}")
    endif()
endforeach()

set(OPENCV_LIBRARIES
    ${OPENCV_LIB_DIR}/libopencv_imgcodecs.so
    ${OPENCV_LIB_DIR}/libopencv_core.so
    ${OPENCV_LIB_DIR}/libopencv_videoio.so
    ${OPENCV_LIB_DIR}/libopencv_imgproc.so
    ${OPENCV_LIB_DIR}/libopencv_highgui.so
    ${OPENCV_LIB_DIR}/libopencv_dnn.so
    ${OPENCV_LIB_DIR}/libopencv_freetype.so
)

# ONNX Runtime
set(ONNX_LIB_DIR ${LIB_DIR}/onnx/1.17.1)
set(ONNX_LIBRARY ${ONNX_LIB_DIR}/libonnxruntime.so)
if(NOT EXISTS "${ONNX_LIBRARY}")
    message(FATAL_ERROR "ONNX Runtime library not found: ${ONNX_LIBRARY}")
endif()

# Kafka configuration
if(KAFKASERVICE OR KAFKAOUTPUT)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLIB REQUIRED glib-2.0)
    pkg_check_modules(RDKAFKA REQUIRED rdkafka)
    pkg_check_modules(RDKAFKAXX REQUIRED rdkafka++)

    if(DEFINED ENV{LIBRDKAFKA_INCLUDE_DIR})
        set(LIBRDKAFKA_INCLUDE_DIR $ENV{LIBRDKAFKA_INCLUDE_DIR})
    else()
        find_file(LIBRDKAFKA_HEADER NAMES rdkafka.h
            HINTS /usr/include/librdkafka ${INC_DIR}/librdkafka /opt/homebrew/include/librdkafka)
        if(NOT LIBRDKAFKA_HEADER)
            message(FATAL_ERROR "Could not find librdkafka headers")
        endif()
        cmake_path(GET LIBRDKAFKA_HEADER PARENT_PATH LIBRDKAFKA_INCLUDE_DIR)
        cmake_path(GET LIBRDKAFKA_INCLUDE_DIR PARENT_PATH LIBRDKAFKA_INCLUDE_DIR)
    endif()

    if(DEFINED ENV{LIBRDKAFKA_LIBRARY_DIR})
        set(LIBRDKAFKA_LIBRARY_DIR $ENV{LIBRDKAFKA_LIBRARY_DIR})
    else()
        find_library(LIBRDKAFKA_LIB NAMES rdkafka
            HINTS /usr/lib ${LIB_DIR} /opt/homebrew/lib)
        if(NOT LIBRDKAFKA_LIB)
            message(FATAL_ERROR "Could not find librdkafka library")
        endif()
        cmake_path(GET LIBRDKAFKA_LIB PARENT_PATH LIBRDKAFKA_LIBRARY_DIR)
    endif()

    message(STATUS "librdkafka include: ${LIBRDKAFKA_INCLUDE_DIR}")
    message(STATUS "librdkafka library: ${LIBRDKAFKA_LIBRARY_DIR}")
endif()

# GPU configuration
if(WITH_GPU)
    add_definitions(-DWITH_GPU)
    set(CUDA_DIR /usr/local/cuda-11.6)
    set(TRT_DIR /usr/local/TensorRT-8.4.0.6)
    include_directories(${CUDA_DIR}/include ${TRT_DIR}/include)
    link_directories(${CUDA_DIR}/lib64 ${TRT_DIR}/lib)
endif()

# Preprocessor definitions
if(BUILDUBUNTU20 OR BUILDUBUNTU22)
    add_definitions(-DCREATEMONGOINSTANCE)
endif()
foreach(opt CLIENTSERVICE WEBSERVICE KAFKASERVICE INSERTDATABASE FAILEDDATABASE STOREIMAGE KAFKAOUTPUT VALUEVALIDATION)
    if(${opt})
        add_definitions(-D${opt})
    endif()
endforeach()

# Include directories
file(GLOB_RECURSE HEADER_FILES "./*.h")
set(HEADER_DIRS "")
foreach(header ${HEADER_FILES})
    get_filename_component(HEADER_DIR ${header} PATH)
    list(APPEND HEADER_DIRS ${HEADER_DIR})
endforeach()
list(REMOVE_DUPLICATES HEADER_DIRS)
include_directories(${HEADER_DIRS})

include_directories(
    ${HEADER_DIRS}
    /home/amiri/programs/rapidjson/include
    ${INC_DIR}/mongocxx/v_noabi
    ${INC_DIR}/bsoncxx/v_noabi
    ${INC_DIR}/bsoncxx/v_noabi/bsoncxx/third_party/mnmlstc
    ${INC_DIR}/libmongoc-1.0
    ${INC_DIR}/libbson-1.0
    ${INC_DIR}/sources
    ${INC_DIR}/sources/nn
    ${Boost_INCLUDE_DIRS}
    ${OPENCV_INCLUDE_DIR}
    ${INC_DIR}/onnx/1.17.1
    ${LIBRDKAFKA_INCLUDE_DIR}
    ${GLIB_INCLUDE_DIRS}
)

# Source files
set(SRC_LIST
    main.cpp
    ocr/ocr.cpp
    Classifier/classifier.cpp
    SystemMonitor/monitor.cpp
    checkop/chop.cpp # TODO: Remove or convert to static library
    ReadConfigurations/configurate.cpp
    Cryptography/cipher.cpp
    Cryptography/cryptotools.cpp
    Cryptography/systemkeys.cpp
    Database/MongoDB.cpp
    Validator/validator.cpp
    Time/timetools.cpp
    Service/Service.cpp
)

if(KAFKASERVICE OR KAFKAOUTPUT)
    list(APPEND SRC_LIST
        Service/KafkaService/kafkaservice.cpp
        Kafka/KafkaProsumer.cpp
    )
endif()
if(CLIENTSERVICE)
    list(APPEND SRC_LIST Service/Rahdari/rahdariService.cpp)
endif()
if(WEBSERVICE)
    list(APPEND SRC_LIST Service/WebService/webservice.cpp)
endif()
if(STOREIMAGE)
    list(APPEND SRC_LIST StoreImage/storeimage.cpp)
endif()
if(INSERTDATABASE OR KAFKAOUTPUT)
    list(APPEND SRC_LIST SaveData/savedata.cpp)
endif()

# Executable
add_executable(${PROJECT_NAME} ${SRC_LIST})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    curl
    pthread
    crypto
    Boost::system
    Boost::filesystem
    Boost::thread
    Boost::chrono
    ${OPENCV_LIBRARIES}
    ${ONNX_LIBRARY}
    /usr/lib/x86_64-linux-gnu/libssl.so.3
    /usr/lib/x86_64-linux-gnu/libcrypto.so.3
    /usr/lib/x86_64-linux-gnu/libfreeimage.so
)

if(KAFKASERVICE OR KAFKAOUTPUT)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${GLIB_LIBRARIES}
        PkgConfig::RDKAFKA
        PkgConfig::RDKAFKAXX
    )
endif()

if(BUILDUBUNTU20)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${LIB_DIR}/libmongocxx.so
        ${LIB_DIR}/libbsoncxx.so
        ${INFERENCELIB_DIR}/libkodnn-20.so
        ${INFERENCELIB_DIR}/libinference-20.so
        # ${LIB_DIR}/libbannerapi.so
    )
elseif(BUILDUBUNTU22)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${LIB_DIR}/mongocxx/libmongocxx.so
        ${LIB_DIR}/mongocxx/libbsoncxx.so
        ${INFERENCELIB_DIR}/libkodnn.so
        ${INFERENCELIB_DIR}/libInference.so
        # ${LIB_DIR}/banner/libbannerapi.so
    )
endif()

# Output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
