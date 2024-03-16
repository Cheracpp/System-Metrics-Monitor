//
// Created by Aymane on 7/10/2023.
//

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "memorymonitor.h"

MemoryMonitor::MemoryMonitor() : stop_thread(false), mem_monitor_thread(&MemoryMonitor::updateMemUsage, this) {}

MemoryMonitor::~MemoryMonitor() {
    stop_thread = true;
    mem_monitor_thread.join();
}

std::vector<std::string> MemoryMonitor::getMemUsage() {
    std::lock_guard<std::mutex> lock(mem_usage_mutex);
    return mem_usages;
}

void MemoryMonitor::updateMemUsage() {

    while (!stop_thread) {
        try {
            std::vector<std::string> newMemUsages = getRawMemData();

            std::lock_guard<std::mutex> lock(mem_usage_mutex);
            mem_usages = newMemUsages;
        } catch (const std::runtime_error &e) {
            std::cerr << "Failed to get memory info: " << e.what() << '\n';
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

std::vector<std::string> MemoryMonitor::getRawMemData() {
    std::vector<std::string> memInfo;
    std::ifstream file("/proc/meminfo");
    if (!file) {
        throw std::runtime_error("Cannot open /proc/meminfo");
    }

    double memTotal = 0, memFree = 0, buffers = 0, cached = 0, swapTotal = 0, swapFree = 0;

    std::string line;
    while (std::getline(file, line)) {
        std::string key = line.substr(0, line.find(':'));
        trim(key);
        if (key == "MemTotal" || key == "MemFree" || key == "Buffers" || key == "Cached" || key == "SwapTotal" ||
            key == "SwapFree") {
            line.erase(line.find(" kB"), 3);
            std::string value_str = line.substr(line.find(':') + 1);
            trim(value_str);
            double value_gb = std::stoi(value_str) / 1024.0 / 1024.0;

            if (key == "MemTotal") memTotal = value_gb;
            else if (key == "MemFree") memFree = value_gb;
            else if (key == "Buffers") buffers = value_gb;
            else if (key == "Cached") cached = value_gb;
            else if (key == "SwapTotal") swapTotal = value_gb;
            else if (key == "SwapFree") swapFree = value_gb;
        }
    }

    // Calculate used memory and swap
    double memUsed = memTotal - memFree - buffers - cached;
    double swapUsed = swapTotal - swapFree;
    std::ostringstream memTotalStr, memUsedStr, swapTotalStr, swapUsedStr;
    memTotalStr << std::fixed << std::setprecision(1) << memTotal;
    memUsedStr << std::fixed << std::setprecision(2) << memUsed;
    swapTotalStr << std::fixed << std::setprecision(2) << swapTotal;
    swapUsedStr << std::fixed << std::setprecision(1) << swapUsed;

    memInfo.push_back("Mem: " + memUsedStr.str() + "/" + memTotalStr.str() + " G");
    memInfo.push_back("SwapTotal: " + swapUsedStr.str() + "/" + swapTotalStr.str() + " G");

    return memInfo;
}

void MemoryMonitor::trim(std::string &str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), str.end());
}
