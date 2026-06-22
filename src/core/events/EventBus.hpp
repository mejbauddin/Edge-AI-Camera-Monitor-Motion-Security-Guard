#pragma once

#include "interfaces/Interfaces.hpp"

#include <mutex>
#include <unordered_map>
#include <vector>

namespace csx::core {

class EventBus final : public IEventBus {
public:
    void subscribe(const std::string& topic, EventCallback callback) override;
    void publish(const std::string& topic, const std::string& payload) override;
    void unsubscribeAll(const std::string& topic) override;

    [[nodiscard]] std::size_t subscriberCount(const std::string& topic) const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::vector<EventCallback>> subscribers_;
};

}  // namespace csx::core
