add_executable(unit-tests
	${PROJECT_SOURCE_DIR}/tests/unit/src/main.cpp
	${PROJECT_SOURCE_DIR}/tests/unit/src/test_socket_communicator.cpp
)

target_link_libraries(unit-tests
	overpass
	gmock
	gmock_main
)
