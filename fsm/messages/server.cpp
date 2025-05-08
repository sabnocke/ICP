
#include "server.h"
#include "zmq.hpp"
#include <iostream>     // For standard I/O
#include <string>       // For using std::string
#include <chrono>       // For std::chrono::seconds
#include <thread>       // For std::this_thread::sleep_for

namespace Server {

int main() {
  zmq::context_t context(1);
  zmq::socket_t socket(context, zmq::socket_type::rep);

  const std::string endpoint = "tcp://*:5555";
  try {
    socket.bind(endpoint);
    std::cout << "Server: Listening on " << endpoint << std::endl;
  } catch (const zmq::error_t& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  while (true) {
    zmq::message_t message;

    try {
      if (auto res = socket.recv(message, zmq::recv_flags::none);
          res.has_value()) {
        std::string received(static_cast<char*>(message.data()), res.value());
        std::cout << received << std::endl;

        std::string resp = "answer";
        zmq::message_t reply(resp.begin(), resp.end());

        socket.send(reply, zmq::send_flags::none);
      }

    } catch (const zmq::error_t& e) {
      std::cerr << e.what() << std::endl;
      break;
    }
  }
  return 0;
}

} // Server