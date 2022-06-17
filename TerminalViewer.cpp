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

std::pair<bool, bool> TerminalViewer::renderUpdate(
    const std::map<
        uint64_t,
        std::map<std::string, std::pair<RingBuffer<float>, StopwatchViewer::TableRow>>>& cache) {
  window_->clear();

  int currRows, currCols = 0;
  Term::get_term_size(currRows, currCols);

  if (currCols != cols_ || currRows != rows_) {
    cols_ = currCols;
    rows_ = currRows;
    window_ = std::make_unique<Term::Window>(cols_, rows_);
  }

  window_->print_border(true);

  const std::u32string leftT = Term::Private::utf8_to_utf32("├");
  const std::u32string rightT = Term::Private::utf8_to_utf32("┤");
  const std::u32string upT = Term::Private::utf8_to_utf32("┴");
  const std::u32string downT = Term::Private::utf8_to_utf32("┬");
  const std::u32string vertical = Term::Private::utf8_to_utf32("│");
  const std::u32string horizontal = Term::Private::utf8_to_utf32("─");
  const std::u32string cross = Term::Private::utf8_to_utf32("┼");
  const std::u32string upArrow = Term::Private::utf8_to_utf32("↑");
  const std::u32string downArrow = Term::Private::utf8_to_utf32("↓");
  const std::u32string upDownArrow = Term::Private::utf8_to_utf32("⇅");

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

  constexpr int kNumColumns = 6;
  bool firstRowVisible = false;
  bool lastRowVisible = false;

  if (cols_ >= 13 && rows_ >= 4) {
    drawHorizontalLine(2);
    drawHorizontalLine(4);

    const int gapBetweenColumns = cols_ / kNumColumns;

    auto drawCharEveryNth = [&](const int row, const auto character) {
      for (int column = 0; column < kNumColumns - 1; column++) {
        window_->set_char(gapBetweenColumns * (column + 1) + 1, row, character);
      }
    };

    // Draw the boundaries
    drawCharEveryNth(2, downT[0]);
    drawCharEveryNth(3, vertical[0]);
    drawCharEveryNth(4, cross[0]);

    for (int row = 5; row < rows_; row++) {
      drawCharEveryNth(row, vertical[0]);
    }

    drawCharEveryNth(rows_, upT[0]);

    // Draw the headers
    const std::array<std::string, 6> headers = {
        "Item", "Last (ms)", "Min (ms)", "Max (ms)", "Avg (ms)", "Hz"};

    auto drawStringsInColumns = [&](const std::array<std::string, 6>& strings, const int row) {
      for (size_t i = 0; i < strings.size(); i++) {
        const int startingX = gapBetweenColumns * i + 2;
        for (int c = 0; c < (int)strings[i].length() && c < gapBetweenColumns - 1; c++) {
          window_->set_char(
              startingX + c, row, Term::Private::utf8_to_utf32(std::string(1, strings[i][c]))[0]);
        }
      }
    };

    drawStringsInColumns(headers, 3);

    // Populate the timing strings
    std::vector<std::array<std::string, 6>> timings;

    for (const auto& [signature, stopwatch] : cache) {
      for (const auto& [name, measurements] : stopwatch) {
        auto& timing = timings.emplace_back();
        timing[0] = name;
        timing[1] = QString::number(measurements.first[0]).toStdString();
        timing[2] = QString::number(measurements.first.getMinimum()).toStdString();
        timing[3] = QString::number(measurements.first.getMaximum()).toStdString();
        timing[4] = QString::number(measurements.first.getAverage()).toStdString();
        timing[5] = QString::number(measurements.first.getReciprocal() * 1000.0).toStdString();
      }
    }

    for (int row = 5, timingIdx = scroll_; row < rows_ && timingIdx < (int)timings.size();
         row++, timingIdx++) {
      if (timingIdx == 0) {
        firstRowVisible = true;
      }

      if (timingIdx == (int)timings.size() - 1) {
        lastRowVisible = true;
      }

      drawStringsInColumns(timings[timingIdx], row);
    }

    if (!firstRowVisible && !lastRowVisible) {
      window_->set_char(cols_ - 1, rows_, upDownArrow[0]);
      window_->set_char(cols_, rows_, Term::Private::utf8_to_utf32(std::string(1, ' '))[0]);
    } else if (!firstRowVisible) {
      window_->set_char(cols_ - 1, rows_, upArrow[0]);
      window_->set_char(cols_, rows_, Term::Private::utf8_to_utf32(std::string(1, ' '))[0]);
    } else if (!lastRowVisible) {
      window_->set_char(cols_ - 1, rows_, downArrow[0]);
      window_->set_char(cols_, rows_, Term::Private::utf8_to_utf32(std::string(1, ' '))[0]);
    }
  }

  std::cout << window_->render(1, 1, true) << std::flush;

  const int key = Term::read_key0();

  bool keepRunning = true;
  bool flushCache = false;

  switch (key) {
    case 'k':
    case Term::Key::ARROW_UP:
      if (!firstRowVisible)
        scroll_--;
      break;
    case 'j':
    case Term::Key::ARROW_DOWN:
      if (!lastRowVisible)
        scroll_++;
      break;
    case 'c':
      flushCache = true;
      break;
    case 'q':
    case Term::Key::ESC:
    case Term::Key::CTRL + 'c':
      keepRunning = false;
      break;
  }

  return std::make_pair(keepRunning, flushCache);
}
