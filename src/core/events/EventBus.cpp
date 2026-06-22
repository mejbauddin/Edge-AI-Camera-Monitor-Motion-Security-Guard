#include "events/EventBus.hpp"
#include "events/EventBusFactory.hpp"

namespace csx::core {

void EventBus::subscribe(const std::string& topic, EventCallback callback) {
    std::lock_guard lock(mutex_);
    subscribers_[topic].push_back(std::move(callback));
}

void EventBus::publish(const std::string& topic, const std::string& payload) {
    std::vector<EventCallback> callbacks;
    {
        std::lock_guard lock(mutex_);
        const auto it = subscribers_.find(topic);
        if (it == subscribers_.end()) {
            return;
        }
        callbacks = it->second;
    }
    for (const auto& callback : callbacks) {
        if (callback) {
            callback(topic, payload);
        }
    }
}

void EventBus::unsubscribeAll(const std::string& topic) {
    std::lock_guard lock(mutex_);
    subscribers_.erase(topic);
}

std::size_t EventBus::subscriberCount(const std::string& topic) const {
    std::lock_guard lock(mutex_);
    const auto it = subscribers_.find(topic);
    return it == subscribers_.end() ? 0 : it->second.size();
}

std::shared_ptr<IEventBus> createEventBus() {
    return std::make_shared<EventBus>();
}

}  // namespace csx::core
