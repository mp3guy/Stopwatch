#include <chrono>
#include <thread>

#include "Stopwatch.h"

int main(int, char**) {
  // This stops duplicate timings on multiple runs
  Stopwatch::getInstance().setCustomSignature(32434);

  while (true) {
    STOPWATCH("Timing1", std::this_thread::sleep_for(std::chrono::milliseconds(10)););

    TICK("Timing2");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    TOCK("Timing2");

    for (size_t i = 3; i < 10; i++) {
      Stopwatch::getInstance().tick(
          "Timing" + std::to_string(i), Stopwatch::getCurrentSystemTime());
      std::this_thread::sleep_for(std::chrono::milliseconds(i));
      Stopwatch::getInstance().tock(
          "Timing" + std::to_string(i), Stopwatch::getCurrentSystemTime());
    }

    Stopwatch::getInstance().sendAll();
  }
}
