//
// Created by ReWyn on 08.05.2025.
//

#include "client.h"

#include <zmq.hpp>
#include <iostream>


namespace ClientMQ {
int main() {
  zmq::context_t context(1);

  zmq::socket_t socket(context, zmq::socket_type::req);
  const std::string endpoint = "tcp://*:5555";
  try {
    socket.connect(endpoint);
    std::cout << "Client: Connecting to " << endpoint << std::endl;
  } catch (zmq::error_t& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
  const std::string request_string = "Hello";
  zmq::message_t reply;
  auto result = socket.recv(reply);

  try {
    if (result.has_value()) {
      std::string received_msg(static_cast<char*>(reply.data()), result.value());
      std::cout << received_msg << std::endl;
    }
  } catch (zmq::error_t& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  return 0;
}

} // ClientMQ