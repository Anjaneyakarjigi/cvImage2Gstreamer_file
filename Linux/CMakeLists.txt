#Love Guru : 40809911
cmake_minimum_required(VERSION 2.8)

project(cvImageToGstreamerFile)

find_package(OpenCV REQUIRED)

find_package(PkgConfig)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)

include_directories(${PROJECT_SOURCE_DIR})

set(cvImageToGstreamerFile OpenCv2GstVideo.cpp)
#set(${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(
        ${GLIB_INCLUDE_DIRS}
        ${GLIB_INCLUDE_DIRS}
        ${GSTREAMER_INCLUDE_DIRS}
)

#linking GStreamer library directory
link_directories(
        ${GLIB_LIBRARY_DIRS}
        ${GSTREAMER_LIBRARY_DIRS}
)

add_executable(cvImageToGstreamerFile ${cvImageToGstreamerFile})

include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LINK_DIRS})

target_link_libraries(cvImageToGstreamerFile ${OpenCV_LIBS} ${GSTREAMER_LIBRARIES})

#cmake -H. -Bbuild -G "Visual Studio 12 2013 Win64" --config Release
#cmake --build build --config Release
