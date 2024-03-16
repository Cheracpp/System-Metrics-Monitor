//
// Created by Aymane on 7/26/2023.
//
#include "processmonitor.h"
#include <dirent.h>
#include <iostream>
#include <cstring>

std::vector<int> ProcessMonitor::getProcessList() const {
    std::vector<int> processList;
    DIR* procDir = opendir("/proc");
    if (procDir == nullptr) {
        std::cerr << "Failed to open /proc directory: " << std::strerror(errno) << std::endl;
        return processList; // Return empty list on failure
    }

    struct dirent* entry;
    while ((entry = readdir(procDir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            std::string name = entry->d_name;
            if (std::all_of(name.begin(), name.end(), ::isdigit)) {
                processList.push_back(std::stoi(name));
            }
        }
    }

    if (closedir(procDir) != 0) {
        std::cerr << "Failed to close /proc directory: " << std::strerror(errno) << std::endl;
    }

    return processList;
}
