#pragma once

#include <cstddef>
#include <list>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace csx::recognition {

class EmbeddingCache {
public:
    explicit EmbeddingCache(std::size_t capacity = 256);

    void put(const std::string& key, const std::vector<float>& embedding);
    [[nodiscard]] std::optional<std::vector<float>> get(const std::string& key) const;
    void clear();

private:
    mutable std::mutex mutex_;
    std::size_t capacity_;
    mutable std::list<std::string> order_;
    mutable std::unordered_map<std::string, std::pair<std::vector<float>, std::list<std::string>::iterator>>
        entries_;
};

}  // namespace csx::recognition
