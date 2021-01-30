cmake_minimum_required(VERSION 3.9)

project(cpp-raster VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

# list source files
set(HEADER_FILES
	raster/include/raster/Pixel.h
	raster/include/raster/Image.h
	raster/include/raster/Vec2i.h
	raster/include/raster/Box.h
	raster/include/raster/Shape.h
	raster/include/raster/Circle.h
	raster/include/raster/Quadtree.h
	raster/include/raster/Swapchain.h
	raster/include/raster/Engine.h
	raster/include/raster/Window.h
)
set(SOURCE_FILES
	raster/src/raster/main.cpp
	raster/src/raster/Quadtree.cpp
	raster/src/raster/Engine.cpp
	raster/src/raster/Window.cpp
)

add_executable(raster
	${SOURCE_FILES} ${HEADER_FILES}
)

add_subdirectory(mn EXCLUDE_FROM_ALL)
target_link_libraries(raster
	PUBLIC
	MoustaphaSaad::mn
)
target_include_directories(raster
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/raster/include
)
