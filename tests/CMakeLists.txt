# Build with system gmock and embedded gtest
set(GMOCK_INCLUDE_DIR "/usr/include/gmock/include" CACHE PATH "gmock source include directory")
set(GMOCK_SOURCE_DIR "/usr/src/gmock" CACHE PATH "gmock source directory")
set(GTEST_INCLUDE_DIR "${GMOCK_SOURCE_DIR}/gtest/include" CACHE PATH "gtest source include directory")

add_subdirectory(${GMOCK_SOURCE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/gmock")

include_directories(
	${GTEST_INCLUDE_DIR}
	${GMOCK_INCLUDE_DIR}
)

add_executable(main_tests
	${PROJECT_SOURCE_DIR}/tests/src/packets/udp_packet_test.cpp
	${PROJECT_SOURCE_DIR}/tests/src/main.cpp
)

target_link_libraries(main_tests
	${Boost_LIBRARIES}
	overpass
	gmock
	gmock_main
)

add_custom_target(run-main-tests
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/main_tests
	DEPENDS main_tests
)

# Custom targets for the tests
add_custom_target(test
	DEPENDS run-main-tests
)
