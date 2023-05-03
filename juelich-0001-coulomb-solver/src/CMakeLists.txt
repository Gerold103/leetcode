add_library(solver
	codec.cpp
	coulomb.cpp
	utils.cpp
)
target_include_directories(solver PUBLIC
	${CMAKE_SOURCE_DIR}/src/
)
