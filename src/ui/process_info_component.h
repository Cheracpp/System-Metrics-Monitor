#ifndef SYSTEM_METRICS_MONITOR_SRC_UI_PROCESS_INFO_COMPONENT_H_
#define SYSTEM_METRICS_MONITOR_SRC_UI_PROCESS_INFO_COMPONENT_H_

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "monitor/process_monitor.h"

class ProcessInfoComponent : public ftxui::ComponentBase {
public:
  explicit ProcessInfoComponent(ftxui::ScreenInteractive *screen);

  ftxui::Element Render() override;
  bool OnEvent(ftxui::Event event) override;

private:
  enum HeaderField {
    kId,
    kUser,
    kPriority,
    kNice,
    kVirtualMemory,
    kResidentMemory,
    kSharedMemory,
    kState,
    kCpuPercentage,
    kMemoryPercentage,
    kTime,
    kCommand
  };

  enum Order { kNaturalOrder, kReverseOrder };

  const char *kUpPointingTriangle = "\u25B2";
  const char *kDownPointingTriangle = "\u25BC";
  const int kMemoryPrecision = 0;
  const int kTimePrecision = 2;
  const int kColumnCount = 12;

  float slider_x_ = 0.f;
  float slider_y_ = 0.f;
  int selected_row_ = 0;
  int percentage_precision_ = 1;
  int header_title_ = HeaderField::kId;
  int order_ = Order::kNaturalOrder;

  std::vector<ftxui::Component> buttons_;
  ftxui::ScreenInteractive *screen_;
  std::vector<int> c_width_;

  ProcessMonitor process_monitor_;

  std::vector<ftxui::Element> FormatProcessData(Process &process);
  void SortData(std::vector<Process> &processes);
  bool NaturalOrder(const Process &a, const Process &b);
  bool ReverseOrder(const Process &a, const Process &b);
};

#endif // SYSTEM_METRICS_MONITOR_SRC_UI_PROCESS_INFO_COMPONENT_H_
