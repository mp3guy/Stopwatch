cmake_minimum_required(VERSION 2.6.0)
project(StopwatchViewer)

find_package(Qt5 COMPONENTS Widgets Network REQUIRED)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses QUIET)

if (CURSES_FOUND)
  add_definitions(-DHAVE_NCURSES)
  include_directories(${CURSES_INCLUDE_DIR})
endif()

add_executable(StopwatchViewer
  main.cpp
  dataPlotWidget.h
  dataPlotWidget.cpp
  ncurses_ui.h
  ncurses_ui.cpp
  plotHolderWidget.h
  plotHolderWidget.cpp
  stopwatchviewer.cpp
  stopwatchviewer.h
  RingBuffer.h
  StopwatchDecoder.h
)

target_link_libraries(StopwatchViewer ${CURSES_LIBRARIES} Qt5::Widgets Qt5::Network)

install(TARGETS StopwatchViewer
        RUNTIME DESTINATION bin)
install(FILES Stopwatch.h DESTINATION include)
