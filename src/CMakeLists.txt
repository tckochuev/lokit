#Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED COMPONENTS Core Gui)

#boost
add_compile_definitions(_HAS_AUTO_PTR_ETC=1)
set(Boost_USE_STATIC_LIBS ON)
#list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/lib/boost")
find_package(Boost 1.62.0 REQUIRED COMPONENTS program_options filesystem)

add_executable(
	${PROJECT_NAME} 
	main.cpp 
	VSLibreOffice.h 
	VSLibreOffice.cpp
	VSUtils.h
)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE 
	${CMAKE_SOURCE_DIR}/lib/LOKit/include
	${Boost_INCLUDE_DIRS}
	${Qt5Core_INCLUDE_DIRS}
	${Qt5Gui_INCLUDE_DIRS}
)

target_link_libraries(
	${PROJECT_NAME} 
	PRIVATE
	${Boost_LIBRARIES}
	Qt5::Core
	Qt5::Gui
)
