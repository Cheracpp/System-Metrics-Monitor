#include "process_monitor.h"
#include "process_worker.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

ProcessMonitor::ProcessMonitor(ftxui::ScreenInteractive *screen)
    : stop_thread_(false) {
  screen_ = screen;
  processes_monitor_thread_ =
      std::thread(&ProcessMonitor::UpdateProcessesInformation, this);
}

ProcessMonitor::~ProcessMonitor() {
  stop_thread_ = true;
  if (processes_monitor_thread_.joinable()) {
    processes_monitor_thread_.join();
  }
}

std::vector<Process> ProcessMonitor::GetProcessesInformation() {
  std::lock_guard<std::mutex> lock(processes_information_mutex_);
  return processes_information_;
}

void ProcessMonitor::UpdateProcessesInformation() {
  while (!stop_thread_) {
    try {
      std::vector<Process> new_processes_information = GetProcessInformation();
      std::lock_guard<std::mutex> lock(processes_information_mutex_);
      processes_information_ = new_processes_information;
    } catch (const std::runtime_error &e) {
      std::cerr << "Failed to get CPU times: " << e.what() << '\n';
    }
    RefreshScreen();
    std::this_thread::sleep_for(std::chrono::milliseconds(kSleepDurationMs));
  }
}

std::vector<Process> ProcessMonitor::GetProcessInformation() {
  std::vector<Process> process_information;
  auto process_set = GetProcessList();
  process_information.reserve(process_set.size());

  for (auto it = previous_cpu_data_.begin(); it != previous_cpu_data_.end();) {
    if (!process_set.contains(it->first)) {
      it = previous_cpu_data_.erase(it);
    } else {
      ++it;
    }
  }

  ProcessWorker::CalculateTotalSystemTicks();

  for (int process_id : process_set) {
    Process process = ProcessWorker::FetchProcessData(process_id);

    if (!previous_cpu_data_.contains(process_id)) {
      previous_cpu_data_[process_id] = {.prev_utime = 0,
                                        .prev_stime = 0,
                                        .prev_total_ticks = 0,
                                        .cpu_percentage = 0.0};
    }

    ProcessWorker::CalculateCpuUsage(process, previous_cpu_data_[process.id]);

    // ignore processes with empty command names
    if (process.command_name.empty() || process.command_name.find_first_not_of(" \t\n\v\f\r") == std::string::npos) {
      continue;
    }
    process_information.push_back(std::move(process));
  }
  return process_information;
}

std::unordered_set<int> ProcessMonitor::GetProcessList() {
  std::unordered_set<int> process_set;
  for (auto const &entry : std::filesystem::directory_iterator("/proc")) {
    if (entry.is_directory()) {
      std::string name = entry.path().filename().string();
      if (std::all_of(name.begin(), name.end(), ::isdigit)) {
        process_set.insert(std::stoi(name));
      }
    }
  }
  return process_set;
}

void ProcessMonitor::RefreshScreen() {
  screen_->PostEvent(ftxui::Event::Custom);
}
