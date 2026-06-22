#pragma once

#include "interfaces/Interfaces.hpp"
#include "pipeline/StageMetrics.hpp"
#include "types/Frame.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <stop_token>
#include <thread>

namespace csx::core {

class PipelineOrchestrator {
public:
    PipelineOrchestrator(std::shared_ptr<ILogger> logger, std::shared_ptr<IEventBus> eventBus);

    void start();
    void stop();
    [[nodiscard]] bool running() const noexcept;

    void setSystemHealth(const SystemHealth& health);
    [[nodiscard]] SystemHealth systemHealth() const;

    StageMetrics& metrics() noexcept { return metrics_; }

private:
    void healthLoop(std::stop_token stopToken);

    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<IEventBus> eventBus_;
    StageMetrics metrics_;
    std::atomic<bool> running_{false};
    mutable std::mutex healthMutex_;
    SystemHealth health_{};
    std::jthread healthThread_;
};

}  // namespace csx::core
