#pragma once

/**
 * This class encapsulates the NCurses viewer. Note that ncurses.h is not
 * included here as it conflicts with Qt definitions.
 */
class NCursesStopwatchViewer {
 public:
  static int run();
};
