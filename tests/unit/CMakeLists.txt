add_executable(unit-tests
	${PROJECT_SOURCE_DIR}/tests/unit/src/main.cpp
)

target_link_libraries(unit-tests
	${Boost_LIBRARIES}
	gmock
	gmock_main
)

add_custom_target(run-unit-tests
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/unit-tests
	DEPENDS unit-tests
)
