#ifndef CPU_MONITOR_H
#define CPU_MONITOR_H

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class CpuMonitor {
 public:
  CpuMonitor();
  ~CpuMonitor();

  const std::vector<std::string>& GetCpuUsages();

 private:
  std::thread cpu_monitor_thread_;
  std::atomic<bool> stop_thread_{false};
  std::mutex cpu_usage_mutex_;
  std::vector<std::string> cpu_usages_;

  static constexpr long kIdleTimeIndex = 3;
  static constexpr long kIOWaitTimeIndex = 4;
  static constexpr long kSleepDurationMs = 1000;

  void UpdateCpuUsage();

  static std::vector<std::string> GetAllCpuUsages();
  static unsigned int GetNumberOfProcessors();
  static std::vector<long> GetCpuTimes(const std::string& cpu_id);
  static double CalculateCpuUsage(const std::vector<long>& times_start, const std::vector<long>& times_end);
  static std::string FormatCpuUsage(unsigned int cpu_num, double usage);
};

#endif  // CPU_MONITOR_H
