#ifndef SYSTEM_METRICS_MONITOR_SRC_UI_MAIN_COMPONENT_H_
#define SYSTEM_METRICS_MONITOR_SRC_UI_MAIN_COMPONENT_H_

#include "ftxui/component/component.hpp"

#include "cpu_component.h"
#include "memory_component.h"
#include "process_info_component.h"

class MainComponent : public ftxui::ComponentBase {
public:
  explicit MainComponent(ftxui::ScreenInteractive *screen);
  ftxui::Element Render() override;
  bool OnEvent(ftxui::Event event) override;

private:
  ftxui::ScreenInteractive *screen_;
  std::shared_ptr<ProcessInfoComponent> process_component_;
  std::shared_ptr<CpuComponent> cpu_component_;
  std::shared_ptr<MemoryComponent> memory_component_;
};

#endif // SYSTEM_METRICS_MONITOR_SRC_UI_MAIN_COMPONENT_H_
