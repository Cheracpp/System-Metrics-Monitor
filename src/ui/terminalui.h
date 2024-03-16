#ifndef TERMINAL_UI_H
#define TERMINAL_UI_H

#include "../monitor/cpumonitor.h"
#include "../monitor/memorymonitor.h"
#include "ftxui/component/component.hpp"  // for CatchEvent, Renderer, operator|=

using namespace ftxui;

class TerminalUI {
public:
    TerminalUI();
    void display();

private:

    CpuMonitor cpu_monitor;
    MemoryMonitor memory_monitor;



    //Components
    Component getCpuUsageComponent();
    Component getMemUsageComponent();
    Component MainComponent(std::function<void()> show_modal, std::function<void()> exit);

};

#endif // TERMINAL_UI_H
