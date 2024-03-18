//
// Created by Aymane on 7/10/2023.
//

#ifndef ACCEPTANCE_PROJECT_MEMORYMONITOR_H
#define ACCEPTANCE_PROJECT_MEMORYMONITOR_H

#include <string>
#include <vector>
#include <mutex>
#include <thread>

class MemoryMonitor {
 public:
  MemoryMonitor();
  ~MemoryMonitor();

  std::vector<std::string> GetMemUsage();

 private:
  std::thread mem_monitor_thread_;
  bool stop_thread_;
  std::mutex mem_usage_mutex_;
  std::vector<std::string> mem_usages_;

  void UpdateMemUsage();

  static std::vector<std::string> GetRawMemData();
  static void Trim(std::string& str);
};

#endif  // ACCEPTANCE_PROJECT_MEMORYMONITOR_H
