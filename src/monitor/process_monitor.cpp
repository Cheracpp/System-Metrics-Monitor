//
// Created by Aymane on 7/26/2023.
//
#include "process_monitor.h"

#include <dirent.h>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

std::vector<int> ProcessMonitor::GetProcessList() const {
  std::vector<int> process_list;
  DIR* proc_dir = opendir("/proc");
  if (proc_dir == nullptr) {
    std::cerr << "Failed to open /proc directory: " << std::strerror(errno) << std::endl;
    return process_list;  // Return empty list on failure.
  }

  struct dirent* entry;
  while ((entry = readdir(proc_dir)) != nullptr) {
    if (entry->d_type == DT_DIR) {
      std::string name = entry->d_name;
      if (std::all_of(name.begin(), name.end(), ::isdigit)) {
        process_list.push_back(std::stoi(name));
      }
    }
  }

  if (closedir(proc_dir) != 0) {
    std::cerr << "Failed to close /proc directory: " << std::strerror(errno) << std::endl;
  }

  return process_list;
}
