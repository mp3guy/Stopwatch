#include <iostream>

#include "TerminalViewer.h"

TerminalViewer::TerminalViewer() : terminal_(true, true, true, true) {
  if (!Term::is_stdin_a_tty()) {
    std::cout << "The terminal is not attached to a TTY and therefore "
                 "can't catch user input. Who knows what happens now?"
              << std::endl;
  }

  Term::get_term_size(rows_, cols_);
  window_ = std::make_unique<Term::Window>(cols_, rows_);
}

void TerminalViewer::renderUpdate() {
  int menuHeight = 10;
  int menuWidth = 10;
  int menuPos = 5;

  window_->clear();
  int menuX0 = (cols_ - menuWidth) / 2 + 1;
  int menuY0 = (rows_ - menuHeight) / 2 + 1;
  window_->print_rect(menuX0, menuY0, menuX0 + menuWidth + 1, menuY0 + menuHeight + 1);

  for (int i = 1; i <= menuHeight; i++) {
    std::string s = std::to_string(i) + ": item";
    window_->print_str(menuX0 + 1, menuY0 + i, s);
    if (i == menuPos) {
      window_->fill_fg(menuX0 + 1, menuY0 + i, menuX0 + s.size(), menuY0 + i, Term::fg::red);
      window_->fill_bg(menuX0 + 1, menuY0 + i, menuX0 + menuWidth, menuY0 + i, Term::bg::gray);
      window_->fill_style(menuX0 + 1, menuY0 + i, menuX0 + s.size(), menuY0 + i, Term::style::bold);
    } else {
      window_->fill_fg(menuX0 + 1, menuY0 + i, menuX0 + s.size(), menuY0 + i, Term::fg::blue);
      window_->fill_bg(menuX0 + 1, menuY0 + i, menuX0 + menuWidth, menuY0 + i, Term::bg::green);
    }
  }

  std::cout << window_->render(1, 1, true) << std::flush;
}
