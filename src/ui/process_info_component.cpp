#include <cmath>
#include <ftxui/component/component.hpp>
#include <iomanip>
#include <sstream>
#include <string>

#include "ftxui/dom/elements.hpp"
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/color.hpp>

#include "process_info_component.h"

ProcessInfoComponent::ProcessInfoComponent(ftxui::ScreenInteractive *screen)
    : process_monitor_(screen), c_width_(12, 0), buttons_(12), screen_(screen) {

  const std::string button_labels[] = {"PID",  "USER", "PRI",   "NI",
                                       "VIRT", "RES",  "SHR",   "S",
                                       "CPU%", "MEM%", "TIME+", "Command"};

  // create buttons that would serve as column titles for the processes table
  for (int c_idx = 0; c_idx < kColumnCount; c_idx++) {
    buttons_[c_idx] = ftxui::Button(
        button_labels[c_idx],
        [&, c_idx] {
          header_title_ = c_idx;
          order_ = (order_ == kNaturalOrder) ? kReverseOrder : kNaturalOrder;
        },
        ftxui::ButtonOption::Ascii());
  }

  // set button sizes
  const int order_icon_size = 3;
  for (int c_idx = 0; c_idx < buttons_.size(); c_idx++) {
    c_width_[c_idx] = button_labels[c_idx].size() + order_icon_size;
  }

  // Add buttons as children under a horizontal container
  Add(ftxui::Container::Horizontal(buttons_));
}

ftxui::Element ProcessInfoComponent::Render() {
  std::vector<Process> processes = process_monitor_.GetProcessesInformation();

  // sort
  SortData(processes);

  // table creation
  std::vector<ftxui::Elements> processed_data;
  processed_data.reserve(processes.size());
  for (auto &process : processes) {
    processed_data.emplace_back(
        FormatProcessData(process)); // inserting formatted processes data
  }

  // Compute the size of each cell
  for (auto &process_data : processed_data) {
    for (int c_idx = 0; c_idx < kColumnCount; ++c_idx) {
      auto &element = process_data[c_idx];
      element->ComputeRequirement();
      c_width_[c_idx] = std::max(c_width_[c_idx], element->requirement().min_x);
    }
  }

  auto create_cell = [&](std::vector<ftxui::Element> &row, ftxui::Element it,
                         int c_idx) -> void {
    if (c_idx != kColumnCount - 1) {
      it = std::move(it) | ftxui::center;
    }
    it = std::move(it) |
         ftxui::size(ftxui::WIDTH, ftxui::EQUAL, c_width_[c_idx]);
    row.push_back(std::move(it));
    row.push_back(ftxui::separatorEmpty());
  };

  std::vector<ftxui::Element> data_rows;
  data_rows.reserve(processed_data.size());
  for (int r_idx = 0; r_idx < (int)processed_data.size(); r_idx++) {
    std::vector<ftxui::Element> row;
    for (int c_idx = 0; c_idx < kColumnCount; c_idx++) {
      create_cell(row, processed_data[r_idx][c_idx], c_idx);
    }

    auto row_box = hbox(std::move(row));
    if (r_idx == selected_row_) {
      row_box = std::move(row_box) | ftxui::color(ftxui::Color::Black) |
                ftxui::bgcolor(ftxui::Color::CadetBlue);
    }
    data_rows.emplace_back(std::move(row_box));
  }

  std::string order_icon =
      (order_ == kNaturalOrder) ? kUpPointingTriangle : kDownPointingTriangle;

  std::vector<ftxui::Element> header_row;
  for (int c_idx = 0; c_idx < buttons_.size(); c_idx++) {
    auto button_box = (c_idx == header_title_) ? hbox(buttons_[c_idx]->Render(),
                                                      ftxui::text(order_icon))
                                               : buttons_[c_idx]->Render();
    create_cell(header_row, button_box, c_idx);
  }

  auto header =
      hbox(std::move(header_row)) | ftxui::color(ftxui::Color::Black) |
      ftxui::bgcolor(ftxui::Color::LightPink1) |
      ftxui::focusPositionRelative(slider_x_, slider_y_) | ftxui::xframe;

  // add slider for vertical movement

  slider_y_ = processes.empty()
                  ? 0.0f
                  : static_cast<float>(selected_row_) / processes.size();

  auto body = vbox(std::move(data_rows)) |
              ftxui::focusPositionRelative(slider_x_, slider_y_) | ftxui::frame | ftxui::flex;

  return vbox(header, body);
}

bool ProcessInfoComponent::OnEvent(ftxui::Event event) {

  // row selection
  if (event == ftxui::Event::ArrowUp || event == ftxui::Event::Character('k')) {
    selected_row_ = std::max(0, selected_row_ - 1);
    return true;
  } else if (event == ftxui::Event::ArrowDown ||
             event == ftxui::Event::Character('j')) {
    selected_row_ =
        std::min((int)process_monitor_.GetProcessesInformation().size() - 1,
                 selected_row_ + 1);
    return true;
  }

  // sliding the table on the x-axis
  if (event == ftxui::Event::ArrowLeft ||
      event == ftxui::Event::Character('h')) {
    slider_x_ = std::max(0.f, slider_x_ - 0.01f);
    return true;
  } else if (event == ftxui::Event::ArrowRight ||
             event == ftxui::Event::Character('l')) {
    slider_x_ = std::min(1.0f, slider_x_ + 0.01f);
    return true;
  }

  // MEM% and CPU% precision modification
  if (event == ftxui::Event::Character('p') && percentage_precision_ > 0) {
    percentage_precision_--;
    return true;
  }
  if (event == ftxui::Event::Character('P') && percentage_precision_ < 6) {
    percentage_precision_++;
    return true;
  }

  return ComponentBase::OnEvent(event);
}

void ProcessInfoComponent::SortData(std::vector<Process> &processes) {
  if (order_ == Order::kNaturalOrder) {
    std::sort(
        processes.begin(), processes.end(),
        [&](const Process &a, const Process &b) { return NaturalOrder(a, b); });
  } else if (order_ == Order::kReverseOrder) {
    std::sort(
        processes.begin(), processes.end(),
        [&](const Process &a, const Process &b) { return ReverseOrder(a, b); });
  }
}

std::vector<ftxui::Element>
ProcessInfoComponent::FormatProcessData(Process &process) {

  auto format_percentage = [&](double &percentage) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(percentage_precision_) << percentage;
    return oss.str();
  };

  auto format_memory = [&](long &memory_bytes) -> std::string {
    const char *units[] = {"B", "K", "M", "G"};
    size_t unit_index = 0;

    auto memory = static_cast<double>(memory_bytes);

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

  auto format_time = [&](double &time) {
    int minutes = time / 60;
    double seconds = std::fmod(static_cast<double>(time), 60.00);
    std::ostringstream oss;
    oss << minutes << ":" << (seconds < 10 ? "0" : "") << std::fixed
        << std::setprecision(kTimePrecision) << seconds;
    return oss.str();
  };
  return {ftxui::text(std::to_string(process.id)),
          ftxui::text(process.user),
          ftxui::text(std::to_string(process.priority)),
          ftxui::text(std::to_string(process.nice_value)),
          ftxui::text(format_memory(process.virtual_memory_byte)),
          ftxui::text(format_memory(process.resident_memory_byte)),
          ftxui::text(format_memory(process.shared_memory_byte)),
          ftxui::text(process.state),
          ftxui::text(format_percentage(process.cpu_usage_percentage)),
          ftxui::text(format_percentage(process.memory_usage_percentage)),
          ftxui::text(format_time(process.total_cpu_time_used_seconds)),
          ftxui::text(process.command_name)

  };
}
bool ProcessInfoComponent::NaturalOrder(const Process &a, const Process &b) {

  switch (header_title_) {
  case HeaderField::kId:
    return a.id < b.id;

  case HeaderField::kUser:
    return std::tie(a.user, a.id) < std::tie(b.user, b.id);

  case HeaderField::kPriority:
    return std::tie(a.priority, a.id) < std::tie(b.priority, b.id);

  case HeaderField::kNice:
    return std::tie(a.nice_value, a.id) < std::tie(b.nice_value, b.id);

  case HeaderField::kVirtualMemory:
    return std::tie(a.virtual_memory_byte, a.id) <
           std::tie(b.virtual_memory_byte, b.id);

  case HeaderField::kResidentMemory:
    return std::tie(a.resident_memory_byte, a.id) <
           std::tie(b.resident_memory_byte, b.id);

  case HeaderField::kSharedMemory:
    return std::tie(a.shared_memory_byte, a.id) <
           std::tie(b.shared_memory_byte, b.id);

  case HeaderField::kState:
    return std::tie(a.state, a.id) < std::tie(b.state, b.id);

  case HeaderField::kCpuPercentage:
    return std::tie(a.cpu_usage_percentage, a.id) <
           std::tie(b.cpu_usage_percentage, b.id);

  case HeaderField::kMemoryPercentage:
    return std::tie(a.memory_usage_percentage, a.id) <
           std::tie(b.memory_usage_percentage, b.id);

  case HeaderField::kTime:
    return std::tie(a.total_cpu_time_used_seconds, a.id) <
           std::tie(b.total_cpu_time_used_seconds, b.id);

  case HeaderField::kCommand:
    return std::tie(a.command_name, a.id) < std::tie(b.command_name, b.id);
  }
  return a.id < b.id;
}

bool ProcessInfoComponent::ReverseOrder(const Process &a, const Process &b) {
  return NaturalOrder(b, a);
}
