#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <sstream>

#include "monitor/memory_monitor.h"
#include "ui/memory_component.h"

MemoryComponent::MemoryComponent() : memory_monitor_() {};

ftxui::Element MemoryComponent::Render() {
  using namespace ftxui;
  auto memory_usage = memory_monitor_.GetMemUsage();

  auto gauge_color = [&](double value) {
    return value < 40   ? color(Color::Green)
           : value < 70 ? color(Color::Yellow)
                        : color(Color::Red);
  };

  auto format_memory = [&](double &memory) -> std::string {
    const char *units[] = {"K", "M", "G"};
    size_t unit_index = 0;

    while (memory >= 1024 &&
           unit_index < sizeof(units) / sizeof(units[0]) - 1) {
      memory /= 1024.0;
      ++unit_index;
    }

    std::ostringstream oss;
    oss.precision(kMemoryPrecision);
    oss << std::fixed << memory << units[unit_index];
    return oss.str();
  };
  auto format_values = [&](double &usage, double &total) {
    std::ostringstream oss;

    oss << format_memory(usage) << "/" << format_memory(total);
    return oss.str();
  };

  auto create__bar = [&](const std::string &label, double usage_value,
                         double total_value) -> Element {
    double percentage = usage_value / total_value * 100.0;
    return hbox({text(label) | color(Color::CadetBlue) | bold,
                 text("[") | color(Color::White),
                 gauge(percentage / 100.0) | gauge_color(percentage),
                 text(format_values(usage_value, total_value)),
                 text("]") | color(Color::White)}) |
           flex;
  };

  return hbox(
      filler(),
      vbox(create__bar("Mem", memory_usage[0].first, memory_usage[0].second),
           create__bar("Swp", memory_usage[1].first, memory_usage[1].second)) |
          flex,
      filler());
}

bool MemoryComponent::OnEvent(ftxui::Event event) {

  return ftxui::ComponentBase::OnEvent(event);
}
