# Build with system gmock and embedded gtest
set(GMOCK_INCLUDE_DIR "/usr/include/gmock/include" CACHE PATH "gmock source include directory")
set(GMOCK_SOURCE_DIR "/usr/src/gmock" CACHE PATH "gmock source directory")
set(GTEST_INCLUDE_DIR "${GMOCK_SOURCE_DIR}/gtest/include" CACHE PATH "gtest source include directory")

add_subdirectory(${GMOCK_SOURCE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/gmock")
include_directories(
	${GTEST_INCLUDE_DIR}
	${GMOCK_INCLUDE_DIR}
)

add_custom_target(test
	DEPENDS run-unit-tests
)

add_subdirectory(unit)
