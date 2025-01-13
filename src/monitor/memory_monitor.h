#ifndef ACCEPTANCE_PROJECT_MEMORY_MONITOR_H_
#define ACCEPTANCE_PROJECT_MEMORY_MONITOR_H_

#include <mutex>
#include <string>
#include <thread>
#include <vector>

class MemoryMonitor {
public:
  MemoryMonitor();
  ~MemoryMonitor();

  std::vector<std::pair<double, double>> GetMemUsage();

private:
  std::thread mem_monitor_thread_;
  bool stop_thread_;
  std::mutex mem_usage_mutex_;
  std::vector<std::pair<double, double>> mem_usages_;

  void UpdateMemUsage();

  static std::vector<std::pair<double, double>> GetRawMemData();
  static void Trim(std::string &str);
};

#endif // ACCEPTANCE_PROJECT_MEMORY_MONITOR_H_
