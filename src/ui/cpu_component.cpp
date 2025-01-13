#include "cpu_component.h"
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <iomanip>
#include <sstream>

CpuComponent::CpuComponent() : cpu_monitor_() {}

ftxui::Element CpuComponent::Render() {
  using namespace ftxui;
  auto usage = cpu_monitor_.GetCpuUsages();
  auto gauge_color = [&](double value) {
    return value < 40   ? color(Color::Green)
           : value < 70 ? color(Color::Yellow)
                        : color(Color::Red);
  };

  auto format_percentage = [this](double percentage) {
    std::ostringstream oss;
    oss << std::setw(kMinimumFieldWidth) << std::fixed
        << std::setprecision(kPrecision) << percentage << "%";
    return oss.str();
  };

  auto create_cpu_bar = [&](const std::string &label,
                            double usage_value) -> Element {
    return hbox({text(label) | color(Color::CadetBlue) | bold,
                 text("[") | color(Color::White),
                 gauge(float(usage_value / 100.0)) | gauge_color(usage_value),
                 text(format_percentage(usage_value)),
                 text("]") | color(Color::White)});
  };

  auto render_cpu_bars = [&](int columns) -> Element {
    std::vector<Element> core_elements;

    for (size_t i = 1; i < usage.size(); ++i) {
      std::ostringstream oss;
      oss << std::setw(2) << (i - 1);
      std::string cpu_number = oss.str();

      core_elements.push_back(create_cpu_bar(cpu_number, usage[i]));
    }

    std::vector<Element> rows;
    for (int i = 0; i < core_elements.size(); i += columns) {
      rows.push_back(
          vbox(std::vector<Element>(
              core_elements.begin() + i,
              core_elements.begin() +
                  std::min(i + columns, (int)core_elements.size()))) |
          flex);
      // separate the columns
      if (i != core_elements.size() - 1)
        rows.push_back(separatorEmpty());
    }

    return hbox(std::move(rows));
  };

  auto overall_cpu = create_cpu_bar(kOverallCpuLabel, usage[0]) | flex;

  return hbox({
      filler(),
      overall_cpu,
      render_cpu_bars(number_of_rows_) | flex_grow,

      filler(),
  });
}
bool CpuComponent::OnEvent(ftxui::Event event) {
  if (event == ftxui::Event::Character('[') && number_of_rows_ < 7) {
    number_of_rows_++;
    return true;
  }
  if (event == ftxui::Event::Character(']') && number_of_rows_ > 3) {
    number_of_rows_--;
    return true;
  }
  return ftxui::ComponentBase::OnEvent(event);
}
