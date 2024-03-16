//
// Created by Aymane on 7/10/2023.
//

#include "terminalui.h"
#include <string> // for operator+, to_string
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/component/event.hpp>  // for Event
#include "ftxui/component/component.hpp"  // for CatchEvent, Renderer, operator|=
#include "ftxui/component/loop.hpp"       // for Loop
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive


TerminalUI::TerminalUI() : cpu_monitor() {}

using namespace ftxui;

void TerminalUI::display() {
    auto screen = ScreenInteractive::Fullscreen();

    auto button_style = ButtonOption::Animated();

    // Modify the way to render them on screen:

    auto exit = screen.ExitLoopClosure();
    auto component10 = Container::Vertical({
                                                   getCpuUsageComponent(),
                                                   getMemUsageComponent(),
                                           });
    Component component = CatchEvent(component10, [&](const Event &event) -> bool {
        if (event == Event::Character("q")) {
            exit();
        }
        return true;
    });

    Loop loop(&screen, component);
    int iteration = 0;
    while (!loop.HasQuitted()) {
        loop.RunOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        iteration++;
        if (iteration % 1 == 0)
            screen.RequestAnimationFrame();
    }
}

Component TerminalUI::getCpuUsageComponent() {
    return Renderer([this] {
        auto elements = cpu_monitor.getCpuUsages();
        // Determine the number of rows and columns.
        int num_elements = elements.size();
        int num_rows = (num_elements + 7) / 8;  // +7 to round up.

        // Create a 2D vector to hold the rows and columns.
        std::vector<std::vector<std::string>> grouped_elements(num_rows, std::vector<std::string>(8, ""));

        // Assign the elements to the 2D vector.
        for (int i = 0; i < num_elements; ++i) {
            int row = i % num_rows;
            int col = i / num_rows;
            grouped_elements[row][col] = elements[i];
        }
        auto table = Table(grouped_elements);

        //table.SelectAll().Border(DOUBLE);
        table.SelectCell(0,0).Decorate(color(Color::Red));

        table.SelectAll().SeparatorVertical();
        table.SelectAll().Decorate(color(Color::White));

        //table.SelectCell(1,1).Decorate(color(Color::Red));
        return table.Render();
    });
}

Component TerminalUI::getMemUsageComponent() {
    return Renderer([this] {
        auto document = memory_monitor.getMemUsage();
        return vbox({
                            vbox({
                                         text(document[0]),
                                         text(document[1]),
                                 })
                    });
    });
}

