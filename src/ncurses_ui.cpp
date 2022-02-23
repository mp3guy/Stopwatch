#include "ncurses_ui.h"

#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <locale>
#include <map>
#include <sstream>
#include <utility>
#include <vector>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <ncurses.h>

#include "RingBuffer.h"
#include "StopwatchDecoder.h"

#include "Stopwatch.h"

class Server {
 public:
  Server(int s) : _socket(s) {}

  ~Server() {
    close(_socket);
  }

  void update() {
    int err = recvfrom(_socket, _recvbuffer, sizeof(_recvbuffer), 0, NULL, 0);
    if (err > 0) {
      updateWithDatagram(_recvbuffer, err);
    }
  }

  static int create(int port, Server** server) {
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
      return s;
    }

    int opt = 1;
    int err = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (err != 0) {
      close(s);
      return err;
    }

    struct sockaddr_in saddr = {0};
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    err = bind(s, reinterpret_cast<struct sockaddr*>(&saddr), sizeof(saddr));
    if (err != 0) {
      close(s);
      return err;
    }

    *server = new Server(s);
    return 0;
  }

  typedef RingBuffer<float, DEFAULT_RINGBUFFER_SIZE> TimingRingBuffer;
  typedef std::map<std::string, TimingRingBuffer> TimingMap;
  typedef unsigned long long int StopwatchIdentifier;
  typedef std::map<StopwatchIdentifier, TimingMap> Stopwatches;

  int _socket;
  Stopwatches _cache;

 private:
  void updateWithDatagram(const unsigned char* buffer, unsigned int length) {
    if (length < sizeof(int)) {
      return;
    }

    const int* data = reinterpret_cast<const int*>(buffer);
    if (length != data[0]) {
      return;
    }

    std::pair<unsigned long long int, std::vector<std::pair<std::string, float>>> currentTimes =
        StopwatchDecoder::decodePacket(reinterpret_cast<const unsigned char*>(buffer), length);
    std::map<std::string, RingBuffer<float, DEFAULT_RINGBUFFER_SIZE>>& stopwatch =
        _cache[currentTimes.first];

    for (unsigned int i = 0; i < currentTimes.second.size(); i++) {
      stopwatch[currentTimes.second.at(i).first].add(currentTimes.second.at(i).second);
    }
  }

  unsigned char _recvbuffer[65536];
};

class Ui {
 public:
  static const int QUIT = 0;
  static const int KEEP_GOING = 1;

  Ui(Server* server) : _window(initscr()), _server(server), _topLineIndex(0) {
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
  }

  ~Ui() {
    endwin();
  }

  int update() {
    char c = getch();
    if (isalpha(c)) {
      c = tolower(c);
    }

    switch (c) {
      case 'q':
        return QUIT;

      case 'f':
        _server->_cache.clear();
        break;

      case 'p':
        _topLineIndex--;
        break;

      case 'n':
        _topLineIndex++;
        break;

      default:
        break;
    }

    draw();

    return KEEP_GOING;
  }

  void draw() {
    _lines.clear();
    Server::Stopwatches::const_iterator stopwatch;
    for (stopwatch = _server->_cache.begin(); stopwatch != _server->_cache.end(); ++stopwatch) {
      Server::TimingMap::const_iterator line;
      for (line = (*stopwatch).second.begin(); line != (*stopwatch).second.end(); ++line) {
        std::ostringstream os;
        os << std::setw(20) << (*line).first << " " << std::setw(6) << (*line).second[0] << " "
           << std::setw(6) << (*line).second.getMinimum() << " " << std::setw(6)
           << (*line).second.getMaximum() << " " << std::setw(6) << (*line).second.getAverage()
           << " " << std::setw(6) << (*line).second.getReciprocal() * 1000.0 << " ";
        _lines.push_back(os.str());
      }
    }

    std::sort(_lines.begin(), _lines.end());

    if (_topLineIndex > _lines.size() - 1) {
      _topLineIndex = _lines.size() - 1;
    }

    if (_topLineIndex < 0) {
      _topLineIndex = 0;
    }

    const char title[] = "StopwatchViewer";
    int center = (COLS - sizeof(title)) / 2;
    attron(A_BOLD | A_UNDERLINE);
    mvprintw(0, center, title);
    attroff(A_BOLD | A_UNDERLINE);

    const char legend[] = "f: flush; n: scroll down, p: scroll up; q: quit";
    center = (COLS - sizeof(legend)) / 2;
    mvprintw(1, center, legend);

    // Each column has 1 space between it
    // Name[20]
    // Last[6]
    // Min[6]
    // Max[6]
    // Avg[6]
    // Hz[6]
    const char header[] = "Name                 Last   Min    Max    Avg    Hz     ";
    attron(A_BOLD);
    mvprintw(3, 0, header);

    // Draw the lines based on the user's scroll position
    std::vector<std::string> lines;

    int output_line = 4;
    int input_line = _topLineIndex;

    while (input_line < _lines.size()) {
      mvprintw(output_line, 0, _lines[input_line].c_str());
      output_line++;
      input_line++;
    }

    refresh();
  }

 private:
  WINDOW* _window;
  Server* _server;
  int _topLineIndex;
  std::vector<std::string> _lines;
};

int NCursesStopwatchViewer::run() {
  Server* server;
  int err = Server::create(45454, &server);
  if (err != 0) {
    return err;
  }

  Ui ui(server);
  ui.draw();

  int retval = 0;
  while (true) {
    fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(server->_socket, &readfds);
    FD_SET(STDIN_FILENO, &readfds);

    int ready = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
    if (ready < 0) {
      retval = ready;
    }

    if (FD_ISSET(server->_socket, &readfds)) {
      server->update();
    }

    if (FD_ISSET(STDIN_FILENO, &readfds)) {
      int disposition = ui.update();
      if (disposition <= 0) {
        retval = disposition;
        break;
      }
    } else {
      ui.draw();
    }
  }

  delete server;
  return retval;
}
