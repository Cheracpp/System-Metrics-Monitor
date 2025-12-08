#include "process_worker.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <pwd.h>
#include <sstream>
#include <unistd.h>

Process ProcessWorker::FetchProcessData(int process_id) {
  Process process{};
  process.id = process_id;

  GetProcessUsername(process);         // user
  GetProcessStateAndPriority(process); // state + nice value + priority + Time
  GetProcessMemoryInfo(process); // virtual, resident + shared memory + MEM%
  GetProcessCommand(process);    // command

  return process;
}

void ProcessWorker::GetProcessUsername(Process &process) {
  const std::string path = "/proc/" + std::to_string(process.id) + "/status";

  std::ifstream pid_status_file(path);
  if (!pid_status_file.is_open()) {
    std::cerr << "Failed to open file: " << path << std::endl;
    return;
  }

  std::string line, uid;
  while (std::getline(pid_status_file, line)) {
    if (line.find("Uid:") == 0) {
      std::istringstream iss(line);
      std::string ignored;
      iss >> ignored >> uid;
      break;
    }
  }

  if (!std::all_of(uid.begin(), uid.end(), ::isdigit)) {
    std::cerr << "uid can't be converted to an integer: " << uid << '\n';
    return;
  }
  struct passwd *pw = getpwuid(std::stoi(uid));
  if (pw && pw->pw_name) {
    process.user = pw->pw_name;
  } else {
    process.user = uid; // Fallback to UID if username not found
  }
}
// nice value + priority + CPU% + Time
void ProcessWorker::GetProcessStateAndPriority(Process &process) {
  const std::string path = "/proc/" + std::to_string(process.id) + "/stat";
  std::ifstream pid_stat_file(path);

  if (!pid_stat_file) {
    std::cerr << "Failed to open file: " << path << '\n';
    return;
  }

  std::string line;
  std::getline(pid_stat_file, line);
  std::istringstream iss(line);

  // Fields to extract
  int process_id;
  std::string comm;
  char state;
  unsigned long utime, stime;
  int priority, nice;
  long cutime, cstime;

  if (!(iss >> process_id)) {
    std::cerr << "Failed to parse PID in " << path << '\n';
    return;
  }

  // Extract `comm` (field 2, enclosed in parentheses)
  if (!(iss >> comm)) {
    std::cerr << "Failed to parse comm in " << path << '\n';
    return;
  }

  // Parse state
  if (!(iss >> state)) {
    std::cerr << "Failed to parse state in " << path << '\n';
    return;
  }

  // Skip fields 4–13
  std::string ignored;
  for (int i = 4; i <= 13; ++i) {
    iss >> ignored;
  }

  // Parse fields 14–19
  if (!(iss >> utime >> stime >> cutime >> cstime >> priority >> nice)) {
    std::cerr << "Failed to parse required fields in " << path << '\n';
    return;
  }

  unsigned long total_ticks = utime + stime;
  long ticks_per_second = sysconf(_SC_CLK_TCK);

  // Convert to total time in seconds
  double total_seconds =
      static_cast<double>(total_ticks) / static_cast<double>(ticks_per_second);

  process.total_cpu_time_used_seconds = total_seconds;
  process.state = state;
  process.nice_value = nice;
  process.priority = priority;
}
void ProcessWorker::GetProcessMemoryInfo(Process &process) {
  const std::string statm_path =
      "/proc/" + std::to_string(process.id) + "/statm";
  const std::string mem_info_path = "/proc/meminfo";

  std::ifstream statm_file(statm_path);
  if (!statm_file) {
    std::cerr << "Failed to open file: " << statm_path << std::endl;
    return;
  }

  const long page_size = sysconf(_SC_PAGESIZE);

  // Read memory metrics from statm
  long total_pages = 0, resident_pages = 0, shared_pages = 0;
  if (statm_file >> total_pages >> resident_pages >> shared_pages) {
    process.virtual_memory_byte = total_pages * page_size;
    process.resident_memory_byte = resident_pages * page_size;
    process.shared_memory_byte = shared_pages * page_size;
  }

  // Calculate memory usage percentage from /proc/meminfo
  std::ifstream meminfo_file(mem_info_path);
  if (meminfo_file) {
    long mem_total_kb = 0;
    std::string line;
    while (std::getline(meminfo_file, line)) {
      if (line.rfind("MemTotal:", 0) == 0) {
        std::istringstream iss(line);
        std::string label;
        iss >> label >> mem_total_kb;
        break;
      }
    }

    if (mem_total_kb > 0) {
      long resident_memory_kb = process.resident_memory_byte / 1024;
      process.memory_usage_percentage =
          (static_cast<double>(resident_memory_kb) /
           static_cast<double>(mem_total_kb)) *
          100.0;
    }
  }
}
void ProcessWorker::GetProcessCommand(Process &process) {
  const std::string path = "/proc/" + std::to_string(process.id) + "/cmdline";
  std::ifstream cmd_file(path);

  if (!cmd_file) {
    std::cerr << "Failed to open file: " << path << std::endl;
    return;
  }

  std::string cmd;
  std::getline(cmd_file, cmd);
  std::replace(cmd.begin(), cmd.end(), '\0', ' ');
  process.command_name = cmd;
}

void ProcessWorker::CalculateCpuUsage(Process &process,
                                      CpuUsageData &prev_data) {
  const std::string stat_path = "/proc/" + std::to_string(process.id) + "/stat";

  std::ifstream stat_file(stat_path);
  if (!stat_file) {
    std::cerr << "Failed to open file: " << stat_path << std::endl;
    return;
  }

  std::string line;
  std::getline(stat_file, line);
  std::istringstream iss(line);

  // Parse necessary fields
  int pid;
  std::string comm;
  char state;
  unsigned long utime, stime;
  long cutime, cstime;

  if (!(iss >> pid >> comm >> state))
    return;

  // Skip fields to reach required data
  for (int i = 3; i < 14; ++i)
    iss.ignore(std::numeric_limits<std::streamsize>::max(), ' ');

  if (!(iss >> utime >> stime >> cutime >> cstime))
    return;

  // Calculate CPU usage
  unsigned long total_process_ticks = utime + stime;

  if (prev_data.prev_total_ticks >
      0) { // Avoid calculation on the first iteration
    unsigned long delta_process_ticks =
        total_process_ticks - (prev_data.prev_utime + prev_data.prev_stime);
    unsigned long delta_total_ticks =
        s_total_system_ticks - prev_data.prev_total_ticks;

    if (delta_total_ticks > 0) {
      process.cpu_usage_percentage =
          (static_cast<double>(delta_process_ticks) / delta_total_ticks) *
          100.0;
    }
  }

  // Update previous data
  prev_data.prev_utime = utime;
  prev_data.prev_stime = stime;
  prev_data.prev_total_ticks = s_total_system_ticks;
}

// Helper function to get total system ticks
void ProcessWorker::CalculateTotalSystemTicks() {
  std::string path = "/proc/stat";
  std::ifstream stat_file(path);
  if (!stat_file) {
    std::cerr << "Failed to open file: " << path << std::endl;
  }

  std::string line;
  std::getline(stat_file, line);
  std::istringstream iss(line);

  std::string cpu_label;
  unsigned long user, nice, system, idle, iowait, irq, softirq;

  if (!(iss >> cpu_label >> user >> nice >> system >> idle >> iowait >> irq >>
        softirq)) {
    std::cerr << "Failed to parse required fields in " << path << '\n';
  }
  s_total_system_ticks = user + nice + system + idle + iowait + irq + softirq;
}

unsigned int ProcessWorker::s_total_system_ticks = 0;
