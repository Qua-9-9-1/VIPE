cmake_minimum_required(VERSION 3.10)

project(My_VIPE)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

find_package(OpenCV REQUIRED)

include_directories(${GTKMM_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

link_directories(${GTKMM_LIBRARY_DIRS} ${OpenCV_LIB_DIRS})

add_definitions(${GTKMM_CFLAGS_OTHER})

add_executable(VIPE src/main.cpp)

target_link_libraries(VIPE ${GTKMM_LIBRARIES} ${OpenCV_LIBS})
