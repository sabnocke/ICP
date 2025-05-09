#pragma once
#include <functional>
#include <thread>

#include "zmq.hpp"

using MessageReceivedCallback =
    std::function<void(std::string_view receivedMessage)>;

class PairCommunicator {
  public:
   PairCommunicator(
       zmq::context_t* context, const std::string& endpoint, bool isBinder,
       std::function<void(std::string_view receivedMessage)> callback);
   ~PairCommunicator();
   void Send(const std::string& message);

  private:
  zmq::context_t &context_;
  zmq::socket_t socket_;
  MessageReceivedCallback &on_message_received_;
  std::thread thread_;
  std::atomic<int> running_;
  void ThreadLoop();

};
