#ifndef CPU_MONITOR_H
#define CPU_MONITOR_H

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>

class CpuMonitor {
public:
    CpuMonitor();
    ~CpuMonitor();

    std::vector<std::string> getCpuUsages();

private:

    std::thread cpu_monitor_thread;
    std::atomic<bool> stop_thread;
    std::mutex cpu_usage_mutex;
    std::vector<std::string> cpu_usages;

    static constexpr long IDLE_TIME_INDEX = 3;
    static constexpr long IOWAIT_TIME_INDEX = 4;
    static constexpr long SLEEP_DURATION_MS = 1000;


    void updateCpuUsage();

    static std::vector<std::string> getAllCpuUsages();
    static unsigned int getNumberOfProcessors();
    static std::vector<long> getCpuTimes(const std::string &cpuId);
    static double calculateCpuUsage(const std::vector<long> &timesStart, const std::vector<long> &timesEnd);

    static std::string formatCpuUsage(unsigned int cpuNum, double usage);
};

#endif // CPU_MONITOR_H
