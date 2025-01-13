#ifndef ACCEPTANCE_PROJECT_PROCESSWORKER_H
#define ACCEPTANCE_PROJECT_PROCESSWORKER_H

#include <string>
#include "process.h"

class ProcessWorker {
 public:
  static Process FetchProcessData(int process_id);
  static void CalculateCpuUsage(Process &process, CpuUsageData &prev_data);
  static void CalculateTotalSystemTicks();

 private:
  static unsigned int s_total_system_ticks;
  static void GetProcessUsername(Process &process);
  static void GetProcessStateAndPriority(Process &process);
  static void GetProcessMemoryInfo(Process &process);
  static void GetProcessCommand(Process &process);
};

#endif  // ACCEPTANCE_PROJECT_PROCESSWORKER_H
