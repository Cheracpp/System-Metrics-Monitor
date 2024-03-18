// Created by Aymane on 7/10/2023.
//

#include "terminal_ui.h"

#include <ftxui/component/event.hpp>  // for Event
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <string>  // for operator+, to_string

#include "ftxui/component/component.hpp"           // for CatchEvent, Renderer, operator|=
#include "ftxui/component/loop.hpp"                // for Loop
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive

using namespace ftxui;

TerminalUI::TerminalUI() : cpu_monitor() {}

void TerminalUI::Display() {
  auto screen = ScreenInteractive::Fullscreen();

  auto container_cpu_and_mem = Container::Vertical({
                                                       GetCpuUsageComponent(),
                                                       GetMemUsageComponent(),
                                                   });

  auto component = CatchEvent(container_cpu_and_mem, [&](const Event& event) {
    if (event == Event::Character('q')) {
      screen.ExitLoopClosure()();
      return true;
    }
    return false;
  });

  Loop loop(&screen, component);

  while (!loop.HasQuitted()) {
    loop.RunOnce();
    std::this_thread::sleep_for(std::chrono::milliseconds(kSleepDurationMs));
    screen.RequestAnimationFrame();
  }
}

Component TerminalUI::GetCpuUsageComponent() {
  return Renderer([this] {
    auto elements = cpu_monitor.GetCpuUsages();

    // Determine the number of rows and columns.
    int num_elements = elements.size();
    int num_rows = (num_elements + 7) / 8;  // +7 to round up.

    // Create a 2D vector to hold the rows and columns.
    std::vector<std::vector<std::string>> grouped_elements(
        num_rows, std::vector<std::string>(8, ""));

    // Assign the elements to the 2D vector.
    for (int i = 0; i < num_elements; ++i) {
      int row = i % num_rows;
      int col = i / num_rows;
      grouped_elements[row][col] = elements[i];
    }

    auto table = Table(grouped_elements);

    table.SelectCell(0, 0).Decorate(color(Color::Red));
    table.SelectAll().SeparatorVertical();
    table.SelectAll().Decorate(color(Color::White));

    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Sleep at the end of the loop
    return table.Render();
  });
}

Component TerminalUI::GetMemUsageComponent() {
  return Renderer([this] {
    auto document = memory_monitor.GetMemUsage();

    return vbox({
                    vbox({
                             text(document[0]),
                             text(document[1]),
                         }),
                });
  });
}
