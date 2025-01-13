#ifndef ACCEPTANCE_PROJECT_PROCESS_MONITOR_H_
#define ACCEPTANCE_PROJECT_PROCESS_MONITOR_H_

#include "process.h"
#include <atomic>
#include <map>
#include <thread>
#include <unordered_set>
#include <vector>

#include <ftxui/component/screen_interactive.hpp>
class ProcessMonitor {
public:
  explicit ProcessMonitor(ftxui::ScreenInteractive *screen_);
  ~ProcessMonitor();

  std::vector<Process> GetProcessesInformation();
  std::map<int, CpuUsageData> previous_cpu_data_;

private:
  static constexpr long kSleepDurationMs = 1000;

  std::thread processes_monitor_thread_;
  std::atomic<bool> stop_thread_;
  std::mutex processes_information_mutex_;
  std::vector<Process> processes_information_;

  ftxui::ScreenInteractive *screen_;

  void RefreshScreen();
  void UpdateProcessesInformation();
  std::vector<Process> GetProcessInformation();
  static std::unordered_set<int> GetProcessList();
};

#endif // ACCEPTANCE_PROJECT_PROCESS_MONITOR_H_
