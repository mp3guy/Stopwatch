#pragma once

#include <map>

#include <cpp-terminal/base.hpp>
#include <cpp-terminal/input.hpp>
#include <cpp-terminal/window.hpp>

#include "ViewerWidget.h"

class TerminalViewer {
 public:
  TerminalViewer();

  std::pair<bool, bool> renderUpdate(
      const std::map<uint64_t, std::map<std::string, std::pair<RingBuffer<float>, TableRow>>>&
          cache);

 private:
  Term::Terminal terminal_;
  std::unique_ptr<Term::Window> window_;

  int rows_ = 0;
  int cols_ = 0;
  int scroll_ = 0;
};
