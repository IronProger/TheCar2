cmake_minimum_required(VERSION 3.5)
project(TheCar)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

#link_directories("/usr/local/lib")

link_libraries(
        opencv_core
        opencv_highgui
        opencv_imgproc
        opencv_video
        opencv_plot
        opencv_video
        opencv_videoio
        opencv_photo
        opencv_xphoto
        opencv_imgcodecs
        stdc++fs
        pthread
        pugixml
)

set(SOURCE_FILES
        main.cpp
        Config.cpp Config.hpp
        Detect.cpp Detect.hpp
        TheCarCV.cpp TheCarCV.hpp
        speedtest.h)

add_executable(TheCar ${SOURCE_FILES})