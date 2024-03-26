cmake_minimum_required(VERSION 3.28)
project(Gabinator_Desktop)
set(OPENCV_DIR "C:/opencv/build")
find_package(OPENCV REQUIRED)

set(SOURCES src/main.cpp )
add_executable(Gabinator_Desktop ${SOURCES})
target_link_libraries( Gabinator_Desktop ${OpenCV_LIBS} )