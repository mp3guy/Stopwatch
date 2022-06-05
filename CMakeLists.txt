cmake_minimum_required(VERSION 3.2)
project(StopwatchViewer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(Qt6 COMPONENTS Widgets Network REQUIRED)
find_package(cpp-terminal REQUIRED)

include_directories(${CPP_TERMINAL_INCLUDE_DIRS})

set(CMAKE_AUTOMOC ON)

file(GLOB sources *.cpp)
file(GLOB headers *.h)

add_executable(StopwatchViewer
  ${sources}
  ${headers}
)

target_link_libraries(StopwatchViewer Qt6::Widgets Qt6::Network ${CPP_TERMINAL_LIBRARIES})

target_compile_options(StopwatchViewer PRIVATE
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Wall>
     $<$<CXX_COMPILER_ID:MSVC>:
          /W4>)

install(TARGETS StopwatchViewer
        RUNTIME DESTINATION bin)
install(FILES Stopwatch.h DESTINATION include)
