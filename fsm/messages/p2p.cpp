// //
// // Created by ReWyn on 08.05.2025.
// //
//
// #include "p2p.h"
// #include <chrono>
// #include <functional>
// #include <iostream>
// #include <string>
// #include <thread>
// #include <vector>
// #include <zmq.hpp>
//
// const std::string endpoint = "ipc://pair.ipc";
//
// PairCommunicator::PairCommunicator(zmq::context_t *context, const std::string &endpoint,
//                    bool isBinder, MessageReceivedCallback callback)
//       : context_(*context),
//         socket_(*context, zmq::socket_type::pair),
//         on_message_received_(callback) {
//   constexpr int receiveTimeout = 100;
//   socket_.set(zmq::sockopt::rcvtimeo, receiveTimeout);
//
//   try {
//     if (isBinder) {
//       socket_.bind(endpoint);
//       std::cout << "[Comm] Bound to " << endpoint << std::endl;
//
//       std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     } else {
//       socket_.connect(endpoint);
//       std::cout << "[Comm] Connect to " << endpoint << std::endl;
//     }
//
//     thread_ = std::thread(&PairCommunicator::ThreadLoop, this);
//   } catch (const zmq::error_t &e) {
//     std::cout << "[Comm] Error: " << e.what() << std::endl;
//     throw;
//   }
// }
//
// void PairCommunicator::ThreadLoop() {
//   while (running_) {
//     zmq::message_t msg;
//     try {
//       auto res = socket_.recv(msg, zmq::recv_flags::none);
//       if (res.has_value()) {
//         std::string_view view(static_cast<char*>(msg.data()), res.value());
//
//         if (on_message_received_) {
//           on_message_received_(view);
//         }
//       } else {
//         // do something
//       }
//     } catch (const zmq::error_t &e) {
//       if (e.num() == ETERM && !running_) {
//         std::cout << "[Comm Thread] Socket terminated during shutdown.\n"; // change to cerr
//         break;
//       } else if (e.num() == EAGAIN) {
//         break;
//       } else {
//         std::cerr << "[Comm Thread] ZeroMQ receive error: " << e.what() << std::endl;
//         break;
//       }
//     } catch (const std::exception &e) {
//       std::cerr << "[Comm Thread] Error: " << e.what() << std::endl;
//       break;
//     }
//   }
//   std::cout << "[Comm Thread] Receive thread exiting.\n";
// }
//
// PairCommunicator::~PairCommunicator() {
//   running_ = false;
//   if (thread_.joinable()) {
//     thread_.join();
//   }
// }
//
// void PairCommunicator::Send(const std::string &message) {
//   try {
//     zmq::message_t msg(message.data(), message.size());
//     socket_.send(msg, zmq::send_flags::none);
//
//   } catch (const zmq::error_t &e) {
//     std::cerr << "[Comm] Error: " << e.what() << std::endl;
//   }
// }
