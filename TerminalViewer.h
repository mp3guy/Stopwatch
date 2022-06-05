#pragma once

#include <cpp-terminal/base.hpp>
#include <cpp-terminal/input.hpp>
#include <cpp-terminal/window.hpp>

class TerminalViewer {
 public:
  TerminalViewer();

  void renderUpdate();

 private:
  Term::Terminal terminal_;
  std::unique_ptr<Term::Window> window_;

  int rows_ = 0;
  int cols_ = 0;
};
