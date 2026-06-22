#include "pipeline/PipelineOrchestrator.hpp"

#include "events/EventTypes.hpp"

#include <nlohmann/json.hpp>
#include <thread>

namespace csx::core {

PipelineOrchestrator::PipelineOrchestrator(std::shared_ptr<ILogger> logger,
                                           std::shared_ptr<IEventBus> eventBus)
    : logger_(std::move(logger)), eventBus_(std::move(eventBus)) {}

void PipelineOrchestrator::start() {
    if (running_.exchange(true)) {
        return;
    }
    if (logger_) {
        logger_->info("PipelineOrchestrator", "AISOS neural pipeline online");
    }

    healthThread_ = std::jthread([this](const std::stop_token stopToken) { healthLoop(stopToken); });
}

void PipelineOrchestrator::stop() {
    if (!running_.exchange(false)) {
        return;
    }
    healthThread_.request_stop();
    if (healthThread_.joinable()) {
        healthThread_.join();
    }
    if (logger_) {
        logger_->info("PipelineOrchestrator", "AISOS neural pipeline offline");
    }
}

bool PipelineOrchestrator::running() const noexcept {
    return running_.load();
}

void PipelineOrchestrator::setSystemHealth(const SystemHealth& health) {
    std::lock_guard lock(healthMutex_);
    health_ = health;
}

SystemHealth PipelineOrchestrator::systemHealth() const {
    std::lock_guard lock(healthMutex_);
    return health_;
}

void PipelineOrchestrator::healthLoop(const std::stop_token stopToken) {
    while (!stopToken.stop_requested() && running_.load()) {
        if (eventBus_) {
            SystemHealth health;
            {
                std::lock_guard lock(healthMutex_);
                health = health_;
            }
            nlohmann::json payload;
            payload["fps"] = health.fps;
            payload["cpu"] = health.cpuPercent;
            payload["ram"] = health.ramPercent;
            eventBus_->publish(events::kSystemHealth, payload.dump());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

}  // namespace csx::core
