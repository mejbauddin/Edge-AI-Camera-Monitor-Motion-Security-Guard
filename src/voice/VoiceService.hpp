#pragma once

#include "interfaces/Interfaces.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace csx::voice {

// ──────────────────────────────────────────────────────────────────────────────
// VoiceSettings — mirrors config/default.json voice.* section
// ──────────────────────────────────────────────────────────────────────────────
struct VoiceSettings {
    bool enabled{true};
    int rate{0};           // SAPI rate (-10 to 10, 0 = normal)
    int volume{90};        // 0 to 100
    int cooldownMs{3000};   // minimum time between same-phrase utterances
};

// ──────────────────────────────────────────────────────────────────────────────
// SpeechQueue — queues speech requests with cooldown management
// ──────────────────────────────────────────────────────────────────────────────
class SpeechQueue {
public:
    struct QueuedSpeech {
        std::string text;
        std::chrono::steady_clock::time_point enqueueTime;
        int priority;  // lower = higher priority
    };

    explicit SpeechQueue(int cooldownMs = 3000);
    ~SpeechQueue() = default;

    void enqueue(const std::string& text, int priority = 0);
    [[nodiscard]] bool hasPending() const;
    [[nodiscard]] QueuedSpeech dequeue();
    void clear();

    void setCooldownMs(int ms) noexcept { cooldownMs_ = ms; }

private:
    mutable std::mutex mutex_;
    std::queue<QueuedSpeech> queue_;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> lastUtterance_;
    int cooldownMs_;
};

// ──────────────────────────────────────────────────────────────────────────────
// VoiceService — Windows SAPI TTS wrapper
// ──────────────────────────────────────────────────────────────────────────────
class VoiceService : public core::IVoiceService {
public:
    explicit VoiceService(VoiceSettings settings = {});
    ~VoiceService() override;

    // IVoiceService
    void speak(const std::string& text, int priority = 0) override;
    void speakAsync(const std::string& text, int priority = 0) override;
    void setEnabled(bool enabled) override;
    [[nodiscard]] bool isEnabled() const noexcept override;
    void stop() override;
    [[nodiscard]] core::EngineHealth health() const override;

    // Control
    void setRate(int rate);
    void setVolume(int volume);
    void setCooldownMs(int ms);

private:
    void workerThread();
    void speakImpl(const std::string& text);

    VoiceSettings settings_;
    std::atomic<bool> enabled_{true};
    std::atomic<bool> running_{true};
    
    SpeechQueue speechQueue_;
    std::thread worker_;
    
    // Windows SAPI interface (opaque pointer to avoid including sapi.h in header)
    void* sapiVoice_;  // ISpVoice*
    
    mutable std::mutex mutex_;
    core::EngineHealth engineHealth_;
};

}  // namespace csx::voice
