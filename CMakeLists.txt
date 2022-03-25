cmake_minimum_required(VERSION 3.14)

project(fpmas-virus)

find_package(fpmas 1.2 REQUIRED)

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
FetchContent_MakeAvailable(yaml-cpp)

target_link_libraries(${PROJECT_NAME} fpmas::fpmas yaml-cpp)

add_executable(seed seed.cpp)
