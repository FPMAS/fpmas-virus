cmake_minimum_required(VERSION 3.14)

project(fpmas-virus VERSION 1.0)

find_package(fpmas 1.5 REQUIRED)

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
