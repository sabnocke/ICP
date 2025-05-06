#pragma once
#include <string>
#include <vector>


namespace AutomatLib {
  class Automat;
  struct Transition;
}

namespace AutomatRuntime {
  class Runtime {
  public:
    std::string Name;
    explicit Runtime(AutomatLib::Automat &automat);
    void ExtractTransitions();
  private:
    AutomatLib::Automat &Automat;
    std::vector<AutomatLib::Transition> transitions;
  };
}





