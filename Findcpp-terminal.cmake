###############################################################################
# Find cpp-terminal
#
# This sets the following variables:
# CPP_TERMINAL_FOUND - True if CPP_TERMINAL was found.
# CPP_TERMINAL_INCLUDE_DIRS - Directories containing the CPP_TERMINAL include files.
# CPP_TERMINAL_LIBRARIES - Libraries needed to use CPP_TERMINAL.

#add a hint so that it can find it without the pkg-config
find_path(CPP_TERMINAL_INCLUDE_DIR cpp-terminal/base.hpp
          PATHS
            "${CMAKE_SOURCE_DIR}/third-party/cpp-terminal/"
)

#add a hint so that it can find it without the pkg-config
find_library(CPP_TERMINAL_LIBRARY
             NAMES cpp-terminal
             PATHS
               "${CMAKE_SOURCE_DIR}/third-party/cpp-terminal/build/cpp-terminal"
)

set(CPP_TERMINAL_INCLUDE_DIRS ${CPP_TERMINAL_INCLUDE_DIR})
set(CPP_TERMINAL_LIBRARIES ${CPP_TERMINAL_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(cpp-terminal DEFAULT_MSG
    CPP_TERMINAL_LIBRARY CPP_TERMINAL_INCLUDE_DIR)

mark_as_advanced(CPP_TERMINAL_LIBRARY CPP_TERMINAL_INCLUDE_DIR)
