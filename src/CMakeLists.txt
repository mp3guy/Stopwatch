cmake_minimum_required(VERSION 2.6.0)
project(StopwatchViewer)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

set(QT_USE_QTNETWORK TRUE)

QT4_WRAP_UI(view_UIS_H stopwatchviewer.ui)

qt4_wrap_cpp(view_moc_srcs
             dataPlotWidget.h
             plotHolderWidget.h
             stopwatchviewer.h
            )

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses QUIET)

if (CURSES_FOUND)
  add_definitions(-DHAVE_NCURSES)
  include_directories(${CURSES_INCLUDE_DIR})
endif()

add_executable(StopwatchViewer 
  main.cpp
  ${view_moc_srcs}
  ${view_UIS_H}
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

target_link_libraries(StopwatchViewer 
                      ${QT_LIBRARIES} 
                      ${QT_QTNETWORK_LIBRARY} ${CURSES_LIBRARIES})

install(TARGETS StopwatchViewer
        RUNTIME DESTINATION bin)
install(FILES Stopwatch.h DESTINATION include)
