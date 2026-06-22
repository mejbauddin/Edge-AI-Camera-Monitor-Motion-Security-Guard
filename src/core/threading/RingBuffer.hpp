#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>

namespace csx::core {

template <typename T, std::size_t Capacity>
class RingBuffer {
    static_assert(Capacity > 0, "RingBuffer capacity must be greater than zero");

public:
    RingBuffer() : head_(0), tail_(0), size_(0) {}

    [[nodiscard]] bool push(T value) {
        const auto currentSize = size_.load(std::memory_order_acquire);
        if (currentSize >= Capacity) {
            return false;
        }

        const auto head = head_.load(std::memory_order_relaxed);
        buffer_[head % Capacity] = std::move(value);
        head_.store((head + 1) % Capacity, std::memory_order_release);
        size_.fetch_add(1, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool pop(T& outValue) {
        const auto currentSize = size_.load(std::memory_order_acquire);
        if (currentSize == 0) {
            return false;
        }

        const auto tail = tail_.load(std::memory_order_relaxed);
        outValue = std::move(buffer_[tail % Capacity]);
        tail_.store((tail + 1) % Capacity, std::memory_order_release);
        size_.fetch_sub(1, std::memory_order_release);
        return true;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return size_.load(std::memory_order_acquire);
    }

    [[nodiscard]] bool empty() const noexcept { return size() == 0; }
    [[nodiscard]] bool full() const noexcept { return size() >= Capacity; }
    [[nodiscard]] static constexpr std::size_t capacity() noexcept { return Capacity; }

    void clear() noexcept {
        head_.store(0, std::memory_order_release);
        tail_.store(0, std::memory_order_release);
        size_.store(0, std::memory_order_release);
    }

private:
    std::array<T, Capacity> buffer_{};
    std::atomic<std::size_t> head_;
    std::atomic<std::size_t> tail_;
    std::atomic<std::size_t> size_;
};

template <typename T>
class SafeQueue {
public:
    explicit SafeQueue(std::size_t maxSize = 1024) : maxSize_(maxSize) {}

    bool push(T value) {
        std::lock_guard lock(mutex_);
        if (queue_.size() >= maxSize_) {
            return false;
        }
        queue_.push_back(std::move(value));
        condition_.notify_one();
        return true;
    }

    bool pop(T& outValue, std::chrono::milliseconds timeout = std::chrono::milliseconds{100}) {
        std::unique_lock lock(mutex_);
        if (!condition_.wait_for(lock, timeout, [this] { return !queue_.empty() || closed_; })) {
            return false;
        }
        if (queue_.empty()) {
            return false;
        }
        outValue = std::move(queue_.front());
        queue_.pop_front();
        return true;
    }

    void close() {
        std::lock_guard lock(mutex_);
        closed_ = true;
        condition_.notify_all();
    }

    [[nodiscard]] std::size_t size() const {
        std::lock_guard lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::deque<T> queue_;
    std::size_t maxSize_;
    bool closed_{false};
};

}  // namespace csx::core
