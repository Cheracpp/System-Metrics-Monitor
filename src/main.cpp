#include "ui/main_component.h"

int main() {
  auto screen = ftxui::ScreenInteractive::Fullscreen();
  ftxui::Component main_component = Make<MainComponent>(&screen);
  screen.Loop(main_component);
  return 0;
}
