# Build with system gmock and embedded gtest
set(GMOCK_INCLUDE_DIR "/usr/include/gmock/include" CACHE PATH "gmock source include directory")
set(GMOCK_SOURCE_DIR "/usr/src/gmock" CACHE PATH "gmock source directory")
set(GTEST_INCLUDE_DIR "${GMOCK_SOURCE_DIR}/gtest/include" CACHE PATH "gtest source include directory")

add_subdirectory(${GMOCK_SOURCE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/gmock")
include_directories(
	${GTEST_INCLUDE_DIR}
	${GMOCK_INCLUDE_DIR}
)

# Custom target to generate the coverage information
add_custom_target(generate-coverage-information
	COMMAND lcov --directory ${PROJECT_BINARY_DIR} --capture --output-file ${PROJECT_BINARY_DIR}/coverage.info
	DEPENDS test
)

# Custom target to filter the coverage information to only relevant files
add_custom_target(filter-coverage-information
	COMMAND lcov --remove ${PROJECT_BINARY_DIR}/coverage.info 'tests/*' '/usr/*' --output-file ${PROJECT_BINARY_DIR}/coverage.info
	DEPENDS generate-coverage-information
)

# Custom target to actually display the coverage information
add_custom_target(coverage
	COMMAND lcov --list ${PROJECT_BINARY_DIR}/coverage.info
	DEPENDS filter-coverage-information
)

# Custom target to run tests
add_custom_target(test
	DEPENDS run-unit-tests
)

add_subdirectory(unit)
