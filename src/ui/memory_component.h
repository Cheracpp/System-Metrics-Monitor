#ifndef SYSTEM_METRICS_MONITOR_SRC_UI_MEMORY_COMPONENT_H_
#define SYSTEM_METRICS_MONITOR_SRC_UI_MEMORY_COMPONENT_H_

#include "monitor/memory_monitor.h"
#include <ftxui/component/component_base.hpp>

class MemoryComponent : public ftxui::ComponentBase {
public:
  MemoryComponent();
  ftxui::Element Render() override;
  bool OnEvent(ftxui::Event event) override;

private:
  const int kMemoryPrecision = 1;
  MemoryMonitor memory_monitor_;
};

#endif // SYSTEM_METRICS_MONITOR_SRC_UI_MEMORY_COMPONENT_H_
