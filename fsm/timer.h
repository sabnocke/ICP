//
// Created by ReWyn on 09.05.2025.
//

#pragma once
#include <chrono>
#include <string>
#include <utility>
#include <vector>
#include <rxcpp/rx.hpp>


struct TimerConfig {
  TimerConfig() = default;
  TimerConfig(const int id, std::string  name, const std::chrono::milliseconds duration) : id(id), name(std::move(name)), duration(duration) {}
  int id{};
  std::string name;
  std::chrono::milliseconds duration{};
};

struct Timer {
  using namespace rxcpp;
private:
  std::vector<TimerConfig> configs;
public:
  void RegisterTimer(const int id, const std::string& name, const std::chrono::milliseconds duration) {
    configs.emplace_back(id, name, duration);
  }

  void Start(std::function dispatch) {
    rxcpp::rxs::iterate(configs, rxcpp::observe_on_new_thread())
    .flat_map([&](const TimerConfig& cfg) {
      return rxcpp::sources::timer(cfg.duration, rxcpp::observe_on_new_thread())
      .map([cfg](long suppress) {
        return cfg;
      });
    })
    .subscribe([](const TimerConfig& cfg) {})
  }

};