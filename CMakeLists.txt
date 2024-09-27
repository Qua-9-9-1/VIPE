cmake_minimum_required(VERSION 3.10)

project(My_VIPE)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

find_package(OpenCV REQUIRED)

include_directories(${GTKMM_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} include)

link_directories(${GTKMM_LIBRARY_DIRS} ${OpenCV_LIB_DIRS})

add_definitions(${GTKMM_CFLAGS_OTHER})
 
file(GLOB_RECURSE SOURCES "src/main.cpp" "src/**/*.cpp")

add_executable(vipe ${SOURCES})

target_link_libraries(vipe ${GTKMM_LIBRARIES} ${OpenCV_LIBS})
