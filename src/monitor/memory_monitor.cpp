//
// Created by Aymane on 7/10/2023.
//

#include "memory_monitor.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>

MemoryMonitor::MemoryMonitor()
    : stop_thread_(false),
      mem_monitor_thread_(&MemoryMonitor::UpdateMemUsage, this) {}

MemoryMonitor::~MemoryMonitor() {
  stop_thread_ = true;
  mem_monitor_thread_.join();
}

std::vector<std::pair<double, double>> MemoryMonitor::GetMemUsage() {
  std::lock_guard<std::mutex> lock(mem_usage_mutex_);
  return mem_usages_;
}

void MemoryMonitor::UpdateMemUsage() {
  while (!stop_thread_) {
    try {
      std::vector<std::pair<double, double>> new_mem_usages = GetRawMemData();

      std::lock_guard<std::mutex> lock(mem_usage_mutex_);
      mem_usages_ = new_mem_usages;
    } catch (const std::runtime_error &e) {
      std::cerr << "Failed to get memory info: " << e.what() << '\n';
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

std::vector<std::pair<double, double>> MemoryMonitor::GetRawMemData() {
  std::vector<std::pair<double, double>> mem_info;
  std::ifstream file("/proc/meminfo");
  if (!file) {
    throw std::runtime_error("Cannot open /proc/meminfo");
  }

  double mem_total = 0, mem_free = 0, buffers = 0, cached = 0, swap_total = 0,
         swap_free = 0;

  std::string line;
  while (std::getline(file, line)) {
    std::string key = line.substr(0, line.find(':'));
    Trim(key);
    if (key == "MemTotal" || key == "MemFree" || key == "Buffers" ||
        key == "Cached" || key == "SwapTotal" || key == "SwapFree") {

      int pos = line.find(':') + 1;
      std::string value_str = line.substr(pos, line.size() - pos - 3);
      Trim(value_str);

      if (!std::all_of(value_str.begin(), value_str.end(), ::isdigit)) {
        std::cerr << "memory value can't be converted to an integer: "
                  << value_str << '\n';
        return mem_info;
      }

      double value_kb = std::stoi(value_str);

      if (key == "MemTotal") {
        mem_total = value_kb;
      } else if (key == "MemFree") {
        mem_free = value_kb;
      } else if (key == "Buffers") {
        buffers = value_kb;
      } else if (key == "Cached") {
        cached = value_kb;
      } else if (key == "SwapTotal") {
        swap_total = value_kb;
      } else if (key == "SwapFree") {
        swap_free = value_kb;
      }
    }
  }

  // Calculate used memory and swap
  double mem_used = mem_total - mem_free - buffers - cached;
  double swap_used = swap_total - swap_free;

  mem_info.push_back({mem_used, mem_total});
  mem_info.push_back({swap_used, swap_total});
  return mem_info;
}

// Remove trailing and leading whitespaces
void MemoryMonitor::Trim(std::string &str) {
  str.erase(str.begin(),
            std::find_if(str.begin(), str.end(),
                         [](unsigned char ch) { return !std::isspace(ch); }));
  str.erase(std::find_if(str.rbegin(), str.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            str.end());
}
