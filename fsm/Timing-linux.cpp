#include "Timing.h"
#include <sys/timerfd.h>
#include <sys/time.h>
#include <iostream>
#include <unistd.h>
#include <string>


namespace Timing {
void Timer::Wait(size_t msTimeout) {
  int stdin = fileno(stdin);

  fd_set read_fds;
  FD_ZERO(&read_fds);
  FD_SET(stdin, &read_fds);

  msTimeout.tv_sec = msTimeout / 1000;
  msTimeout.tv_usec = (msTimeout % 1000) * 1000;

  int ready = select(stdin + 1, &read_fds, NULL, NULL, &msTimeout);

  if (ready == -1) {
    perror("select (Unix)");
    return;
  } else if (ready == 0) {
    std::cerr << "Timeout (Unix)" << std::endl;
    //TODO handle timeout
  } else {
    if (FD_ISSET(stdin, &read_fds)) {
      std::cout << "Data available on stdin (Unix):\n";
      std::string input;
      std::getline(std::cin, input);
      std::cout << "Received: " << input << "\n";
    }
  }
}
}  // namespace Timing