//
// Created by Aymane on 7/10/2023.
//

#include "memory_monitor.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

MemoryMonitor::MemoryMonitor()
    : stop_thread_(false),
      mem_monitor_thread_(&MemoryMonitor::UpdateMemUsage, this) {}

MemoryMonitor::~MemoryMonitor() {
  stop_thread_ = true;
  mem_monitor_thread_.join();
}

std::vector<std::string> MemoryMonitor::GetMemUsage() {
  std::lock_guard<std::mutex> lock(mem_usage_mutex_);
  return mem_usages_;
}

void MemoryMonitor::UpdateMemUsage() {
  while (!stop_thread_) {
    try {
      std::vector<std::string> new_mem_usages = GetRawMemData();

      std::lock_guard<std::mutex> lock(mem_usage_mutex_);
      mem_usages_ = new_mem_usages;
    } catch (const std::runtime_error& e) {
      std::cerr << "Failed to get memory info: " << e.what() << '\n';
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

std::vector<std::string> MemoryMonitor::GetRawMemData() {
  std::vector<std::string> mem_info;
  std::ifstream file("/proc/meminfo");
  if (!file) {
    throw std::runtime_error("Cannot open /proc/meminfo");
  }

  double mem_total = 0, mem_free = 0, buffers = 0, cached = 0, swap_total = 0, swap_free = 0;

  std::string line;
  while (std::getline(file, line)) {
    std::string key = line.substr(0, line.find(':'));
    Trim(key);
    if (key == "MemTotal" || key == "MemFree" || key == "Buffers" || key == "Cached" ||
        key == "SwapTotal" || key == "SwapFree") {
      line.erase(line.find(" kB"), 3);
      std::string value_str = line.substr(line.find(':') + 1);
      Trim(value_str);
      double value_gb = std::stoi(value_str) / 1024.0 / 1024.0;

      if (key == "MemTotal") {
        mem_total = value_gb;
      } else if (key == "MemFree") {
        mem_free = value_gb;
      } else if (key == "Buffers") {
        buffers = value_gb;
      } else if (key == "Cached") {
        cached = value_gb;
      } else if (key == "SwapTotal") {
        swap_total = value_gb;
      } else if (key == "SwapFree") {
        swap_free = value_gb;
      }
    }
  }

  // Calculate used memory and swap
  double mem_used = mem_total - mem_free - buffers - cached;
  double swap_used = swap_total - swap_free;

  // Format memory usage
  std::ostringstream mem_total_str, mem_used_str, swap_total_str, swap_used_str;
  mem_total_str << std::fixed << std::setprecision(1) << mem_total;
  mem_used_str << std::fixed << std::setprecision(2) << mem_used;
  swap_total_str << std::fixed << std::setprecision(2) << swap_total;
  swap_used_str << std::fixed << std::setprecision(1) << swap_used;

  mem_info.push_back("Mem: " + mem_used_str.str() + "/" + mem_total_str.str() + " G");
  mem_info.push_back("Swap: " + swap_used_str.str() + "/" + swap_total_str.str() + " G");

  return mem_info;
}

void MemoryMonitor::Trim(std::string& str) {
  str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
    return !std::isspace(ch);
  }));
  str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
    return !std::isspace(ch);
  }).base(), str.end());
}
