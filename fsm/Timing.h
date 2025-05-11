///
///! This is just broken (at least the rxcpp) if used on ucrt
///TODO change this to only start one timer, the one with smallest duration
///? In case of conflict pick at random or dunno
///

#pragma once

#include <chrono>
#include <condition_variable>
#include <string>
#include <vector>
#include "types/transitions.h"
#include <Windows.h>

namespace Timing {

using namespace types;

struct Timer {
private:

public:
  std::string Wait(size_t msTimeout);

};
}