cmake_minimum_required(VERSION 3.8)

project("CoulombSolver")

set(CMAKE_CXX_STANDARD 17)

add_compile_options(
	-Wall -Wextra -Wpedantic -Werror -Wno-unknown-warning-option -Wunused-function
	-Wno-invalid-offsetof -Wno-unused-value -Wno-deprecated-copy
	-Wno-gnu-zero-variadic-macro-arguments
	-fno-permissive
	-fno-exceptions
	-fno-rtti
)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

add_subdirectory(src)
add_subdirectory(test)

add_executable(solver_exe
	main.cpp
)
target_include_directories(solver_exe PUBLIC
	${CMAKE_SOURCE_DIR}/src/
)
target_link_libraries(solver_exe
	solver
)
