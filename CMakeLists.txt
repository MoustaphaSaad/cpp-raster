cmake_minimum_required(VERSION 3.9)

# list source files
set(SOURCE_FILES
	raster/src/main.cpp
)

add_executable(raster
	${SOURCE_FILES}
)

add_subdirectory(mn/mn)
target_link_libraries(raster
	MoustaphaSaad::mn
)
