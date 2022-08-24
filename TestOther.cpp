#include <chrono>
#include <thread>

#include "Stopwatch.h"

int main(int, char**) {
  // This stops duplicate timings on multiple runs
  Stopwatch::getInstance().setCustomSignature(2434);

  while (true) {
    TICK("Timing10");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    Stopwatch::getInstance().sendAll();
  }
}
