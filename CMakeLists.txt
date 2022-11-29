cmake_minimum_required(VERSION 3.14)

project(fpmas-virus VERSION 1.0)

find_package(fpmas 1.6 REQUIRED)

include_directories(include)
add_executable(${PROJECT_NAME}
	main.cpp
	src/agent.cpp
	src/output.cpp
	)

include(FetchContent)
FetchContent_Declare(
	yaml-cpp
	GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
	GIT_TAG        yaml-cpp-0.7.0
	)
FetchContent_Declare(
	cli11
	GIT_REPOSITORY https://github.com/CLIUtils/CLI11
	GIT_TAG        v2.2.0
	)

FetchContent_MakeAvailable(yaml-cpp cli11)

target_link_libraries(${PROJECT_NAME} fpmas::fpmas
	yaml-cpp::yaml-cpp
	CLI11::CLI11)

add_executable(seed seed.cpp)

# Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs)
	set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs)
	set(DOXYGEN_HTML_OUTPUT ".")
	set(DOXYGEN_SORT_MEMBER_DOCS "NO")
	set(DOXYGEN_HAVE_DOT "YES")
	set(DOXYGEN_HIDE_FRIEND_COMPOUNDS "YES")
	set(DOXYGEN_EXCLUDE_SYMBOLS [[YAML::*]])
	
	doxygen_add_docs(doc
		${CMAKE_CURRENT_SOURCE_DIR}/include
		)
endif()
