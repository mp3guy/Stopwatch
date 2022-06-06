#include <iostream>

#include <cpp-terminal/private/conversion.hpp>

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

bool TerminalViewer::renderUpdate() {
  window_->clear();

  int currRows, currCols = 0;
  Term::get_term_size(currRows, currCols);

  if (currCols != cols_ || currRows != rows_) {
    cols_ = currCols;
    rows_ = currRows;
    window_ = std::make_unique<Term::Window>(cols_, rows_);
  }

  window_->print_border(true);

  int menuHeight = 10;
  int menuWidth = 10;
  int menuPos = 5;

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

  const std::u32string leftT = Term::Private::utf8_to_utf32("├");
  const std::u32string rightT = Term::Private::utf8_to_utf32("┤");
  const std::u32string upT = Term::Private::utf8_to_utf32("┴");
  const std::u32string downT = Term::Private::utf8_to_utf32("┬");
  const std::u32string vertical = Term::Private::utf8_to_utf32("│");
  const std::u32string horizontal = Term::Private::utf8_to_utf32("─");

  // Print title
  const std::string title = "StopwatchViewer";
  if (cols_ >= (int)title.length()) {
    window_->print_str(cols_ / 2 - title.length() / 2 + 1, 1, title);
  }

  auto drawHorizontalLine = [&](const int row) {
    window_->set_char(1, row, leftT[0]);
    for (int x = 2; x < cols_; x++) {
      window_->set_char(x, row, horizontal[0]);
    }
    window_->set_char(cols_, row, rightT[0]);
  };

  drawHorizontalLine(2);
  drawHorizontalLine(4);

  constexpr int kNumColumns = 6;

  if (cols_ >= 13) {
    const int gapBetweenColumns = cols_ / kNumColumns;

    auto drawCharEveryNth = [&](const int row, const auto character) {
      for (int column = 0; column < kNumColumns - 1; column++) {
        window_->set_char(gapBetweenColumns * (column + 1) + 1, row, character);
      }
    };

    drawCharEveryNth(3, vertical[0]);
    drawCharEveryNth(2, downT[0]);
    drawCharEveryNth(4, upT[0]);
  }

  std::cout << window_->render(1, 1, true) << std::flush;

  const int key = Term::read_key0();

  switch (key) {
    case 'q':
    case Term::Key::ESC:
    case Term::Key::CTRL + 'c':
      return false;
      break;
  }

  return true;
}
