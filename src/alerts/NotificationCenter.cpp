#include "NotificationCenter.hpp"

#include <algorithm>

namespace csx::alerts {

void NotificationCenter::subscribe(AlertCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    subscribers_.push_back(std::move(callback));
}

void NotificationCenter::unsubscribe(const AlertCallback& callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    subscribers_.erase(
        std::remove_if(subscribers_.begin(), subscribers_.end(),
                      [&callback](const AlertCallback& cb) {
                          // Compare by target (assuming function objects are comparable)
                          return cb.target<void(*)(const Alert&)>() == 
                                 callback.target<void(*)(const Alert&)>();
                      }),
        subscribers_.end());
}

void NotificationCenter::notify(const Alert& alert) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& callback : subscribers_) {
        callback(alert);
    }
}

void NotificationCenter::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    subscribers_.clear();
}

size_t NotificationCenter::subscriberCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return subscribers_.size();
}

}  // namespace csx::alerts
