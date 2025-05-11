#include "Timing.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include <Windows.h>

namespace Timing {

  std::string Timer::Wait(size_t msTimeout) {
    HANDLE timerHandle = CreateWaitableTimer(nullptr, TRUE, nullptr);
    if (timerHandle == nullptr) {
      std::cout << "CreateWaitableTimer failed!" << std::endl;
      return "";
    }

    LARGE_INTEGER dueTime;
    dueTime.QuadPart = -static_cast<LONGLONG>(msTimeout) * 10000;
    if (!SetWaitableTimer(timerHandle, &dueTime, 0, nullptr, nullptr, FALSE)) {
      std::cout << "SetWaitableTimer failed!" << std::endl;
      CloseHandle(timerHandle);
      return "";
    }

    HANDLE stdinHandle = GetStdHandle(STD_INPUT_HANDLE);
    std::vector<HANDLE> handlesToWait;

    handlesToWait.push_back(timerHandle);
    handlesToWait.push_back(stdinHandle);


    DWORD waitResult = WaitForMultipleObjects(handlesToWait.size(), handlesToWait.data(), FALSE, INFINITE);

    if (waitResult == WAIT_OBJECT_0) {
      std::cout << "Timer expired!" << std::endl;
      return "TIMER";
    } if (waitResult == WAIT_OBJECT_0 + 1) {
      std::cout << "Data available on stdin!" << std::endl;
      std::string data;
      std::getline(std::cin, data);
      std::cout << data << std::endl;
      return data;
    } if (waitResult == WAIT_FAILED) {
      std::cerr << "WaitForMultipleObjects failed: " << GetLastError() << std::endl;
      return "";
    }
    std::cerr << "Unknown wait result: " << waitResult << std::endl;
    return "";
  }

}