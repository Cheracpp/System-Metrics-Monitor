//
// Created by Aymane on 7/10/2023.
//

#include "cpumonitor.h"
#include <fstream>
#include <sstream>
#include <numeric>
#include <iomanip>
#include <iostream>

CpuMonitor::CpuMonitor() : stop_thread(false) {
    // Initialize cpu_usages with 0% usage
    unsigned int numProcessors = getNumberOfProcessors();
    cpu_usages.push_back(formatCpuUsage(-1, 0.0));

    for (unsigned int i = 0; i < numProcessors; ++i) {
        cpu_usages.push_back(formatCpuUsage(i, 0.0));
    }

    cpu_monitor_thread = std::thread(&CpuMonitor::updateCpuUsage, this);
}

CpuMonitor::~CpuMonitor() {
    stop_thread = true;
    if(cpu_monitor_thread.joinable()){
        cpu_monitor_thread.join();
    }
}

std::vector<std::string> CpuMonitor::getCpuUsages() {
    std::lock_guard<std::mutex> lock(cpu_usage_mutex);
    return cpu_usages;
}

void CpuMonitor::updateCpuUsage() {
    while (!stop_thread) {
        try {
            std::vector<std::string> newCpuUsages = getAllCpuUsages();
            std::lock_guard<std::mutex> lock(cpu_usage_mutex);
            cpu_usages = newCpuUsages;
        } catch (const std::runtime_error &e) {
            std::cerr << "Failed to get CPU times: " << e.what() << '\n';
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // sleep at the end of the loop
    }
}

std::vector<std::string> CpuMonitor::getAllCpuUsages() {
    std::vector<std::string> allCpuUsages;

    //get i times
    std::vector<long> overallTimesStart = getCpuTimes("cpu");
    unsigned int numProcessors = getNumberOfProcessors();
    std::vector<std::vector<long>> timesStart(numProcessors);
    for (unsigned int i = 0; i < numProcessors; ++i) {
        std::string cpuId = "cpu" + std::to_string(i);
        timesStart[i] = getCpuTimes(cpuId);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION_MS));

    std::vector<long> overallTimesEnd = getCpuTimes("cpu");
    double overallCpuUsage = calculateCpuUsage(overallTimesStart, overallTimesEnd);
    allCpuUsages.push_back(formatCpuUsage(-1, overallCpuUsage));

    for (unsigned int i = 0; i < numProcessors; ++i) {
        std::string cpuId = "cpu" + std::to_string(i);
        std::vector<long> timesEnd = getCpuTimes(cpuId);
        double cpuUsage = calculateCpuUsage(timesStart[i], timesEnd);
        allCpuUsages.push_back(formatCpuUsage(i, cpuUsage));
    }

    return allCpuUsages;
}

unsigned int CpuMonitor::getNumberOfProcessors() {
    return std::thread::hardware_concurrency();
}

std::vector<long> CpuMonitor::getCpuTimes(const std::string &cpuId) {
    std::vector<long> times;
    std::ifstream procStat("/proc/stat");
    if (!procStat) {
        throw std::runtime_error("Cannot open /proc/stat");
    }

    std::string line;
    while (std::getline(procStat, line)) {
        if (line.compare(0, cpuId.size(), cpuId) == 0) {
            std::istringstream iss(line);
            std::string ignore;
            iss >> ignore;
            long time;
            while (iss >> time)
                times.push_back(time);
            return times;
        }
    }
    throw std::runtime_error("Cannot find cpuId in /proc/stat");
}

double CpuMonitor::calculateCpuUsage(const std::vector<long> &timesStart, const std::vector<long> &timesEnd) {
    if (timesStart.size() < 5 || timesEnd.size() < 5) {
        throw std::runtime_error("Invalid CPU times");
    }

    long idleTimeStart = timesStart[IDLE_TIME_INDEX] + timesStart[IOWAIT_TIME_INDEX];
    long idleTimeEnd = timesEnd[IDLE_TIME_INDEX] + timesEnd[IOWAIT_TIME_INDEX];
    long totalTimeStart = std::accumulate(timesStart.begin(), timesStart.end(), 0L);
    long totalTimeEnd = std::accumulate(timesEnd.begin(), timesEnd.end(), 0L);
    long deltaIdle = idleTimeEnd - idleTimeStart;
    long deltaTotal = totalTimeEnd - totalTimeStart;
    if (deltaTotal == 0)
        return 0;
    return (static_cast<double>(deltaTotal) - static_cast<double>(deltaIdle)) * 100.0 / static_cast<double>(deltaTotal);
}

std::string CpuMonitor::formatCpuUsage(unsigned int cpuNum, double usage) {
    std::ostringstream stream;
    if (cpuNum == -1) {
        stream << "Overall CPU: ";
    } else {
        stream << "CPU" << cpuNum << ": ";
    }
    stream << std::fixed << std::setprecision(1) << usage << "%";
    return stream.str();
}
