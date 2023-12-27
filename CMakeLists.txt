cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-g)

project(Aggregation LANGUAGES CXX)

OPTION(BUILDUBUNTU22 "Build in Ubuntu22" ON)
OPTION(BUILDUBUNTU20 "Build in Ubuntu20" OFF)
OPTION(WEBSERVICE "Run Web service" ON)
OPTION(KAFKASERVICE "Run Kafka service" ON)

if(BUILDUBUNTU22)
    add_definitions(-DCREATEMONGOINSTANCE)
endif()

if(WEBSERVICE)
    add_definitions(-DWEBSERVICE)
endif()

if(KAFKASERVICE)
    add_definitions(-DKAFKASERVICE)
endif()

include_directories(.)

if(BUILDUBUNTU20)
    include_directories(/home/mohammad/opencv/4.2.0/include/opencv4)
    set(OPENCVLIB_dir /usr/local/lib)
elseif(BUILDUBUNTU22)
    include_directories(/home/mohammad/opencv/4.7/include/opencv4)
    set(OPENCVLIB_dir /usr/local/lib/opencv)
endif()

include_directories(.
                    /usr/local/include/mongocxx/v_noabi
                    /usr/local/include/bsoncxx/v_noabi
                    /usr/local/include/bsoncxx/v_noabi/bsoncxx/third_party/mnmlstc
                    /usr/local/include/libmongoc-1.0
                    /usr/local/include/libbson-1.0)

set(OPENCVLIB       ${OPENCVLIB_dir}/libopencv_imgcodecs.so
                    ${OPENCVLIB_dir}/libopencv_core.so
                    ${OPENCVLIB_dir}/libopencv_videoio.so
                    ${OPENCVLIB_dir}/libopencv_imgproc.so
                    ${OPENCVLIB_dir}/libopencv_highgui.so
                    ${OPENCVLIB_dir}/libopencv_dnn.so
                    ${OPENCVLIB_dir}/libopencv_freetype.so
    
                    ${OPENCVLIB_dir}/libopencv_aruco.so 
                    ${OPENCVLIB_dir}/libopencv_bgsegm.so
                    ${OPENCVLIB_dir}/libopencv_bioinspired.so
                    ${OPENCVLIB_dir}/libopencv_calib3d.so
                    ${OPENCVLIB_dir}/libopencv_ccalib.so
                    ${OPENCVLIB_dir}/libopencv_datasets.so
                    ${OPENCVLIB_dir}/libopencv_dnn_objdetect.so
                    ${OPENCVLIB_dir}/libopencv_dnn.so
                    ${OPENCVLIB_dir}/libopencv_dnn_superres.so
                    ${OPENCVLIB_dir}/libopencv_dpm.so
                    ${OPENCVLIB_dir}/libopencv_face.so
                    ${OPENCVLIB_dir}/libopencv_features2d.so
                    ${OPENCVLIB_dir}/libopencv_flann.so
                    ${OPENCVLIB_dir}/libopencv_fuzzy.so
                    ${OPENCVLIB_dir}/libopencv_gapi.so
                    ${OPENCVLIB_dir}/libopencv_hdf.so
                    ${OPENCVLIB_dir}/libopencv_hfs.so
                    ${OPENCVLIB_dir}/libopencv_img_hash.so
                    ${OPENCVLIB_dir}/libopencv_line_descriptor.so
                    ${OPENCVLIB_dir}/libopencv_ml.so
                    ${OPENCVLIB_dir}/libopencv_objdetect.so
                    ${OPENCVLIB_dir}/libopencv_optflow.so
                    ${OPENCVLIB_dir}/libopencv_phase_unwrapping.so
                    ${OPENCVLIB_dir}/libopencv_photo.so
                    ${OPENCVLIB_dir}/libopencv_plot.so
                    ${OPENCVLIB_dir}/libopencv_quality.so
                    ${OPENCVLIB_dir}/libopencv_reg.so
                    ${OPENCVLIB_dir}/libopencv_rgbd.so
                    ${OPENCVLIB_dir}/libopencv_saliency.so
                    ${OPENCVLIB_dir}/libopencv_shape.so
                    ${OPENCVLIB_dir}/libopencv_stereo.so
                    ${OPENCVLIB_dir}/libopencv_stitching.so
                    ${OPENCVLIB_dir}/libopencv_structured_light.so
                    ${OPENCVLIB_dir}/libopencv_superres.so
                    ${OPENCVLIB_dir}/libopencv_surface_matching.so
                    ${OPENCVLIB_dir}/libopencv_text.so
                    ${OPENCVLIB_dir}/libopencv_tracking.so
                    ${OPENCVLIB_dir}/libopencv_video.so
                    ${OPENCVLIB_dir}/libopencv_videostab.so
                    ${OPENCVLIB_dir}/libopencv_xfeatures2d.so
                    ${OPENCVLIB_dir}/libopencv_ximgproc.so
                    ${OPENCVLIB_dir}/libopencv_xobjdetect.so
                    ${OPENCVLIB_dir}/libopencv_xphoto.so)

if(KAFKASERVICE)
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
             Database/MongoDB.cpp)

if(KAFKASERVICE)
    set(SRC_LIST    ${SRC_LIST}
                    Service/KafkaService/kafkaservice.cpp
                    Kafka/KafkaProsumer.cpp)
endif()
if(WEBSERVICE)
    set(SRC_LIST    ${SRC_LIST}
                    Service/WebService/webservice.cpp)
endif()

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} 
                      pthread 
                      crypto 
                      boost_thread 
                      boost_chrono 
                      ${OPENCVLIB})

if(KAFKASERVICE)
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