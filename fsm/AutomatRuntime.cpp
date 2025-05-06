//
// Created by ReWyn on 06.05.2025.
//

#include "AutomatRuntime.h"

#include "AutomatLib.h"
namespace AutomatRuntime {
   Runtime::Runtime(AutomatLib::Automat &automat) : Automat(automat) {
     Name = automat.Name;
   }

  void Runtime::ExtractTransitions() {

  }



}