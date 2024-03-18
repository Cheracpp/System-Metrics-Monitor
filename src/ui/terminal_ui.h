#ifndef TERMINAL_UI_H
#define TERMINAL_UI_H

#include "../monitor/cpu_monitor.h"
#include "../monitor/memory_monitor.h"
#include "ftxui/component/component.hpp"  // for CatchEvent, Renderer, operator|=

using namespace ftxui;

class TerminalUI {
 public:
  TerminalUI();
  void Display();

 private:
  CpuMonitor cpu_monitor;
  MemoryMonitor memory_monitor;

  static constexpr long kSleepDurationMs = 17;

  // Components
  Component GetCpuUsageComponent();
  Component GetMemUsageComponent();
  Component MainComponent(std::function<void()> show_modal, std::function<void()> exit);
};

#endif  // TERMINAL_UI_H
