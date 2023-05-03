add_executable(test_exe
	test.cpp
)
target_include_directories(test_exe PUBLIC
	${CMAKE_SOURCE_DIR}/src/
)
target_link_libraries(test_exe
	solver
)
