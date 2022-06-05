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

    Stopwatch::getInstance().sendAll();
  }
}
