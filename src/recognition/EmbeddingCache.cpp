#include "EmbeddingCache.hpp"

namespace csx::recognition {

EmbeddingCache::EmbeddingCache(const std::size_t capacity) : capacity_(capacity == 0 ? 1 : capacity) {}

void EmbeddingCache::put(const std::string& key, const std::vector<float>& embedding) {
    std::lock_guard lock(mutex_);
    const auto existing = entries_.find(key);
    if (existing != entries_.end()) {
        order_.erase(existing->second.second);
        entries_.erase(existing);
    }

    order_.push_front(key);
    entries_[key] = {embedding, order_.begin()};

    while (entries_.size() > capacity_) {
        const auto& oldestKey = order_.back();
        entries_.erase(oldestKey);
        order_.pop_back();
    }
}

std::optional<std::vector<float>> EmbeddingCache::get(const std::string& key) const {
    std::lock_guard lock(mutex_);
    const auto it = entries_.find(key);
    if (it == entries_.end()) {
        return std::nullopt;
    }
    return it->second.first;
}

void EmbeddingCache::clear() {
    std::lock_guard lock(mutex_);
    entries_.clear();
    order_.clear();
}

}  // namespace csx::recognition
