#pragma once

#include "AlertManager.hpp"

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace csx::alerts {

// ──────────────────────────────────────────────────────────────────────────────
// NotificationCenter — distributes alerts to UI and other subscribers
// ──────────────────────────────────────────────────────────────────────────────
class NotificationCenter {
public:
    using AlertCallback = std::function<void(const Alert&)>;
    
    NotificationCenter() = default;
    ~NotificationCenter() = default;
    
    void subscribe(AlertCallback callback);
    void unsubscribe(const AlertCallback& callback);
    
    void notify(const Alert& alert);
    
    void clear();
    [[nodiscard]] size_t subscriberCount() const;

private:
    mutable std::mutex mutex_;
    std::vector<AlertCallback> subscribers_;
};

}  // namespace csx::alerts
