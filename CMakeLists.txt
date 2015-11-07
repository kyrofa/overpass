cmake_minimum_required(VERSION 2.6)
project(liboverpass)

# Version number
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

configure_file(
	${PROJECT_SOURCE_DIR}/data/version.h.in
	${PROJECT_BINARY_DIR}/include/version.h
)

include_directories(
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_BINARY_DIR}/include
)

find_package(Boost REQUIRED COMPONENTS
	system
	program_options
)
include_directories(${Boost_INCLUDE_DIRS})

set(OVERPASS_HEADERS
	${PROJECT_SOURCE_DIR}/include/udp_packet.h
	${PROJECT_SOURCE_DIR}/include/virtual_interface.h
)

set(OVERPASS_SOURCES
	${PROJECT_SOURCE_DIR}/src/udp_packet.cpp
	${PROJECT_SOURCE_DIR}/src/virtual_interface.cpp
)

add_definitions("-std=c++11 -Wall -Wextra")

add_library(overpass
	${OVERPASS_HEADERS}
	${OVERPASS_SOURCES}
)

add_executable(overpassd
	${OVERPASS_HEADERS}
	${PROJECT_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(overpassd
	overpass
	${Boost_LIBRARIES}
	pthread
)

set(BUILD_TESTS true CACHE BOOL "Whether or not to build the tests")
if(BUILD_TESTS)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0") # Debug, no optimization
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage") # Enable coverage
	add_subdirectory(tests)
endif()
