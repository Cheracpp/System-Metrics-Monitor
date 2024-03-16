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

    std::vector<std::string> getMemUsage();

private:
    std::thread mem_monitor_thread;
    bool stop_thread;
    std::mutex mem_usage_mutex;
    std::vector<std::string> mem_usages;

    void updateMemUsage();

    static std::vector<std::string> getRawMemData();
    static void trim(std::string& str);
};


#endif //ACCEPTANCE_PROJECT_MEMORYMONITOR_H
