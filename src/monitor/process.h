#ifndef SYSTEM_METRICS_MONITOR_SRC_MONITOR_PROCESS_H_
#define SYSTEM_METRICS_MONITOR_SRC_MONITOR_PROCESS_H_

#include <string>

struct Process {
  int id;
  std::string user;
  int priority;
  int nice_value;
  long virtual_memory_byte;
  long resident_memory_byte;
  long shared_memory_byte;
  std::string state;
  double cpu_usage_percentage = 0.0;
  double memory_usage_percentage;
  double total_cpu_time_used_seconds;
  std::string command_name;
};

struct CpuUsageData {
  unsigned long prev_utime = 0;
  unsigned long prev_stime = 0;
  unsigned long prev_total_ticks = 0;
  double cpu_percentage = 0.0;
};

#endif // SYSTEM_METRICS_MONITOR_SRC_MONITOR_PROCESS_H_
