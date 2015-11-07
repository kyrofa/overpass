# Build with system gmock and embedded gtest
set(GMOCK_INCLUDE_DIR "/usr/include/gmock/include" CACHE PATH "gmock source include directory")
set(GMOCK_SOURCE_DIR "/usr/src/gmock" CACHE PATH "gmock source directory")
set(GTEST_INCLUDE_DIR "${GMOCK_SOURCE_DIR}/gtest/include" CACHE PATH "gtest source include directory")

add_subdirectory(${GMOCK_SOURCE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/gmock")
include_directories(
	${GTEST_INCLUDE_DIR}
	${GMOCK_INCLUDE_DIR}
)

# Custom target to generate, merge, and filter coverage reports after the test.
add_custom_target(coverage
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	COMMAND lcov -c -d . -o tested.info
	COMMAND lcov -a baseline.info -a tested.info -o coverage.info
	COMMAND lcov --remove coverage.info 'tests/*' '/usr/*' -o coverage.info
	COMMAND lcov --list coverage.info
	DEPENDS test
)

# Custom target to generate an empty initial coverage report so we get uncovered files.
# Add new test targets onto the DEPENDS.
add_custom_target(pre-test
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	COMMAND lcov -c -i -d . -o baseline.info
	DEPENDS overpass overpassd run-unit-tests
)

# Custom target to run tests
add_custom_target(test
	DEPENDS pre-test
)

add_subdirectory(unit)
