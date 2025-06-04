#include <chrono>
#include <thread>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <stdexcept>

// Assuming these are defined elsewhere
#include "Service.h"
// #include "Logger.h"

namespace Monitoring {

struct Metrics {
    uint64_t chopCounter{0};
    uint64_t chopTime{0};
    uint64_t storeImageCounter{0};
    uint64_t saveRecordCounter{0};
    uint64_t storeImageTime{0};
    uint64_t saveRecordTime{0};
    uint64_t queueSize{0};
    uint64_t nullImageCount{0};
    uint64_t imageCount{0};
    uint64_t imageRequestCount{0};
    uint64_t infoRequestCount{0};
    uint64_t infoRequestTime{0};
    uint64_t imageRequestTime{0};
};

class SystemMonitor {
private:
    static constexpr int MONITOR_INTERVAL_SECONDS = 20;
    static constexpr const char* LOG_FILE_PATH = "/var/log/chop_monitor.log";
    std::mutex logMutex;
    std::ofstream logFile;

    std::string getTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    Metrics collectMetrics() {
        Metrics metrics;
        metrics.chopCounter = Service::chopCounter.exchange(0);
        metrics.chopTime = Service::chopTime.exchange(0);
        metrics.storeImageCounter = Service::storeImageCounter.exchange(0);
        metrics.saveRecordCounter = Service::saveRecordCounter.exchange(0);
        metrics.storeImageTime = Service::storeImageTime.exchange(0);
        metrics.saveRecordTime = Service::saveRecordTime.exchange(0);
        metrics.queueSize = Service::queueSize;
        metrics.nullImageCount = Service::nullImageCounter.exchange(0);
        metrics.imageCount = Service::ImageCounter.exchange(0);
        metrics.imageRequestCount = Service::ImageRequestCounter.exchange(0);
        metrics.infoRequestCount = Service::InfoRequestCounter.exchange(0);
        metrics.infoRequestTime = Service::InfoRequestTime.exchange(0);
        metrics.imageRequestTime = Service::ImageRequestTime.exchange(0);
        return metrics;
    }

    void logMetrics(const Metrics& metrics) {
        uint64_t infoRequestTimeAvg = metrics.infoRequestCount > 0 
            ? metrics.infoRequestTime / metrics.infoRequestCount : 0;
        uint64_t imageRequestTimeAvg = metrics.imageRequestCount > 0 
            ? metrics.imageRequestTime / metrics.imageRequestCount : 0;
        uint64_t chopTimeAvg = metrics.chopCounter > 0 
            ? metrics.chopTime / metrics.chopCounter : 0;

        uint64_t storeImageTimeAvg = metrics.storeImageCounter > 0 
            ? metrics.storeImageTime / metrics.storeImageCounter : 0;
            
        uint64_t saveRecordTimeAvg = metrics.saveRecordCounter > 0 
            ? metrics.saveRecordTime / metrics.saveRecordCounter : 0;

        std::stringstream logMessage;
        std::string timestamp = getTimestamp();
        
        logMessage << "[" << timestamp << "] MONITORING_INTERVAL\n"
                    << "chop_counter=" << metrics.chopCounter << "\n"
                    << "chop_rate_per_sec=" << (metrics.chopCounter / MONITOR_INTERVAL_SECONDS) << "\n"
                    << "queue_size=" << metrics.queueSize << "\n"
                    << "null_image_count=" << metrics.nullImageCount << "\n"
                    << "image_count=" << metrics.imageCount << "\n"
                    << "image_request_count=" << metrics.imageRequestCount << "\n"
                    << "info_request_count=" << metrics.infoRequestCount << "\n"
                    << "info_request_time_ms=" << metrics.infoRequestTime << "\n"
                    << "image_request_time_ms=" << metrics.imageRequestTime << "\n"
                    << "chop_time_ms=" << metrics.chopTime << "\n"
                    << "info_request_time_avg_ms=" << infoRequestTimeAvg << "\n"
                    << "image_request_time_avg_ms=" << imageRequestTimeAvg << "\n"
                    << "chop_time_avg_ms=" << chopTimeAvg << "\n"
                    << "storeImage_time_avg_microseconds=" << storeImageTimeAvg << "\n"
                    << "savedatabase_time_avg_microseconds=" << saveRecordTimeAvg << "\n"
                    << "----------------------------------------\n";

        {
            std::lock_guard<std::mutex> lock(logMutex);
            if (!logFile.is_open()) {
                logFile.open(LOG_FILE_PATH, std::ios::app);
                if (!logFile.is_open()) {
                    throw std::runtime_error("Failed to open log file: " + std::string(LOG_FILE_PATH));
                }
            }
            logFile << logMessage.str();
            logFile.flush();
        }

        Logger::getInstance().logImportant2(logMessage.str());
    }

public:
    SystemMonitor() = default;
    ~SystemMonitor() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void monitor() {
        while (true) {
            try {
                std::this_thread::sleep_for(std::chrono::seconds(MONITOR_INTERVAL_SECONDS));
                
                Metrics metrics = collectMetrics();
                logMetrics(metrics);
            }
            catch (const std::exception& e) {
                Logger::getInstance().logImportant2("Monitoring error: " + std::string(e.what()));
                continue;
            }
        }
    }
};

void startMonitoring() {
    static SystemMonitor monitor;
    std::thread monitorThread(&SystemMonitor::monitor, &monitor);
    monitorThread.detach();
}

} // namespace Monitoring