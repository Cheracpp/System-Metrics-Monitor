#include "ftxui/component/component.hpp"
#include "ftxui/component/loop.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include "main_component.h"

MainComponent::MainComponent(ftxui::ScreenInteractive *screen)
    : screen_(screen) {
  Add(cpu_component_);
  Add(process_component_);
  Add(memory_component_);
};

ftxui::Element MainComponent::Render() {
  using namespace ftxui;
  return vbox({
      separatorEmpty(),
      cpu_component_->Render(),
      separatorEmpty(),
      memory_component_->Render(),
      separatorEmpty(),
      process_component_->Render(),
  });
}

bool MainComponent::OnEvent(ftxui::Event event) {
  if (event == ftxui::Event::Character('q')) {
    screen_->Exit();
    return true;
  }
  return ComponentBase::OnEvent(event);
}
