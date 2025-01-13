#ifndef SYSTEM_METRICS_MONITOR_SRC_UI_CPU_COMPONENT_H_
#define SYSTEM_METRICS_MONITOR_SRC_UI_CPU_COMPONENT_H_

#include "monitor/cpu_monitor.h"
#include <ftxui/component/component_base.hpp>

class CpuComponent : public ftxui::ComponentBase {
public:
  CpuComponent();

  ftxui::Element Render() override;
  bool OnEvent(ftxui::Event event) override;

private:
  const char *kOverallCpuLabel = "\u2211";
  const int kPrecision = 1;
  // the maximum width that the string percentage can have is its precision + 4
  // (3 digits "100", comma ',')
  const int kMinimumFieldWidth = kPrecision + 4;

  int number_of_rows_ = 3;
  CpuMonitor cpu_monitor_;
};

#endif // SYSTEM_METRICS_MONITOR_SRC_UI_CPU_COMPONENT_H_
