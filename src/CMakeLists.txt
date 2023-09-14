add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE 
	${CMAKE_SOURCE_DIR}/lib/LOKit/include
	${CMAKE_SOURCE_DIR}/lib/boost
	${Qt5Core_INCLUDE_DIRS}
	${Qt5Gui_INCLUDE_DIRS}
)

target_link_libraries(
	${PROJECT_NAME} 
	PRIVATE 
	Qt5::Core
	Qt5::Gui
)
