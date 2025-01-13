//
// Created by Aymane on 7/10/2023.
//

#include "cpu_monitor.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

CpuMonitor::CpuMonitor() : stop_thread_(false) {
  // Initialize CPU usage with 0% usage for each processor
  unsigned int num_processors = GetNumberOfProcessors();
  cpu_usages_.push_back(0.0);

  for (unsigned int i = 0; i < num_processors; ++i) {
    cpu_usages_.push_back(0.0);
  }

  cpu_monitor_thread_ = std::thread(&CpuMonitor::UpdateCpuUsage, this);
}

CpuMonitor::~CpuMonitor() {
  stop_thread_ = true;
  if (cpu_monitor_thread_.joinable()) {
    cpu_monitor_thread_.join();
  }
}

const std::vector<double> &CpuMonitor::GetCpuUsages() {
  std::lock_guard<std::mutex> lock(cpu_usage_mutex_);
  return cpu_usages_;
}

void CpuMonitor::UpdateCpuUsage() {
  while (!stop_thread_) {
    try {
      std::vector<double> new_cpu_usages = GetAllCpuUsages();
      std::lock_guard<std::mutex> lock(cpu_usage_mutex_);
      cpu_usages_ = new_cpu_usages;
    } catch (const std::runtime_error &e) {
      std::cerr << "Failed to get CPU times: " << e.what() << '\n';
    }
  }
}

std::vector<double> CpuMonitor::GetAllCpuUsages() {
  std::vector<double> all_cpu_usages;

  // Gather start times
  std::vector<long> overall_times_start = GetCpuTimes("cpu");
  unsigned int num_processors = GetNumberOfProcessors();
  std::vector<std::vector<long>> times_start(num_processors);

  for (unsigned int i = 0; i < num_processors; ++i) {
    std::string cpu_id = "cpu" + std::to_string(i);
    times_start[i] = GetCpuTimes(cpu_id);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(kSleepDurationMs));

  // Gather end times and calculate usage
  std::vector<long> overall_times_end = GetCpuTimes("cpu");
  double overall_cpu_usage =
      CalculateCpuUsage(overall_times_start, overall_times_end);
  all_cpu_usages.push_back(overall_cpu_usage);

  for (unsigned int i = 0; i < num_processors; ++i) {
    std::string cpu_id = "cpu" + std::to_string(i);
    std::vector<long> times_end = GetCpuTimes(cpu_id);
    double cpu_usage = CalculateCpuUsage(times_start[i], times_end);
    all_cpu_usages.push_back(cpu_usage);
  }

  return all_cpu_usages;
}

unsigned int CpuMonitor::GetNumberOfProcessors() { return std::thread::hardware_concurrency(); }

std::vector<long> CpuMonitor::GetCpuTimes(const std::string &cpu_id) {
  std::vector<long> times;
  std::ifstream proc_stat("/proc/stat");
  if (!proc_stat) {
    throw std::runtime_error("Cannot open /proc/stat");
  }

  std::string line;
  while (std::getline(proc_stat, line)) {
    if (line.compare(0, cpu_id.size(), cpu_id) == 0) {
      std::istringstream iss(line);
      std::string ignore;
      iss >> ignore;
      long time;
      while (iss >> time) {
        times.push_back(time);
      }
      return times;
    }
  }

  throw std::runtime_error("Cannot find CPU ID in /proc/stat");
}

double CpuMonitor::CalculateCpuUsage(const std::vector<long> &times_start,
                                     const std::vector<long> &times_end) {
  if (times_start.size() < 5 || times_end.size() < 5) {
    throw std::runtime_error("Invalid CPU times");
  }

  long idle_time_start =
      times_start[kIdleTimeIndex] + times_start[kIOWaitTimeIndex];
  long idle_time_end = times_end[kIdleTimeIndex] + times_end[kIOWaitTimeIndex];

  long total_time_start =
      std::accumulate(times_start.begin(), times_start.end(), 0L);
  long total_time_end = std::accumulate(times_end.begin(), times_end.end(), 0L);

  long delta_idle = idle_time_end - idle_time_start;
  long delta_total = total_time_end - total_time_start;

  if (delta_total == 0) {
    return 0.0;
  }

  return (static_cast<double>(delta_total) - static_cast<double>(delta_idle))
      * 100.0 /
      static_cast<double>(delta_total);
}

std::string CpuMonitor::FormatCpuUsage(unsigned int cpu_num, double usage) {
  std::ostringstream stream;
  if (cpu_num == static_cast<unsigned int>(-1)) {
    stream << "Overall CPU: ";
  } else {
    stream << "CPU" << cpu_num << ": ";
  }
  stream << std::fixed << std::setprecision(1) << usage << "%";
  return stream.str();
}
