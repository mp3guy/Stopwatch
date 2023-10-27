Stopwatch
=========

Easy to use simple benchmarking tool.

Sends UDP packets to localhost, which StopwatchViewer receives and displays stats on, including plots (credit to [Fiachra Matthews](http://www.linkedin.com/pub/fiachra-matthews/17/48b/a12) here).

StopwatchViewer needs Qt6 to build. Simply include Stopwatch.h in whatever code you want to benchmark and use as such;

```cpp
#include <string>
#include <stdio.h>
#include "Stopwatch.h"

int main(int argc, char *argv[])
{
  //This stops duplicate timings on multiple runs
  Stopwatch::getInstance().setCustomSignature(32434);

  STOPWATCH("Timing1",

  if(argc >= 1)
  {
    sleep(1);
  }

  );


  TICK("Timing2");

  while(argc > 0)
  {
    sleep(2);
    argc--;
  }

  TOCK("Timing2");

  Stopwatch::getInstance().sendAll();
}
```

Then just watch the stats in StopwatchViewer GUI. Pass in the "-c" flag to use the CLI instead of Qt GUI. 

Uses some code from the B-Human code release (http://www.b-human.de/).

<p align="center">
  <img src="https://github.com/mp3guy/mp3guy.github.io/raw/master/img/Stopwatch.png" alt="Stopwatch"/>
</p>

<p align="center">
  <img src="https://github.com/mp3guy/mp3guy.github.io/raw/master/img/StopwatchCLI.png" alt="StopwatchCLI"/>
</p>

Install
=======
```
git clone  https://github.com/mp3guy/Stopwatch.git
cd Stopwatch
git submodule init && git submodule update
cd third-party/cpp-terminal
mkdir build && cd build
cmake .. && make -j 
cd ../../
mkdir build && cd build
cmake .. && make -j
```
Running Stopwatch in terminal mode without GUI
===============================================
```
./Stopwatch -c
```
With key bindings: 
- k or up arrow: scroll up
- j or down arrow: scroll down
- c: flush the cache
- q or Esc or Ctrl+c: quit
