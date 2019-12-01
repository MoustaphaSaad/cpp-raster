cmake_minimum_required(VERSION 3.9)

project(cpp-raster VERSION 0.1.0 LANGUAGES CXX)

add_subdirectory(mn EXCLUDE_FROM_ALL)
add_subdirectory(fabric/fabric EXCLUDE_FROM_ALL)

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

target_link_libraries(raster
	PUBLIC
	MoustaphaSaad::mn
	MoustaphaSaad::fabric
)
target_include_directories(raster
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/raster/include
)
