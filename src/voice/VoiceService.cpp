#include "VoiceService.hpp"

#include <sapi.h>
#include <windows.h>

#include <iostream>
#include <unordered_map>

namespace csx::voice {

// ══════════════════════════════════════════════════════════════════════════════
// SpeechQueue
// ══════════════════════════════════════════════════════════════════════════════

SpeechQueue::SpeechQueue(int cooldownMs) : cooldownMs_(cooldownMs) {}

void SpeechQueue::enqueue(const std::string& text, int priority) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    const auto now = std::chrono::steady_clock::now();
    const auto it = lastUtterance_.find(text);
    
    // Check cooldown
    if (it != lastUtterance_.end()) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - it->second).count();
        if (elapsed < cooldownMs_) {
            return;  // Skip due to cooldown
        }
    }
    
    queue_.push({text, now, priority});
}

bool SpeechQueue::hasPending() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return !queue_.empty();
}

SpeechQueue::QueuedSpeech SpeechQueue::dequeue() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
        return {"", std::chrono::steady_clock::time_point{}, 999};
    }
    auto item = queue_.front();
    queue_.pop();
    lastUtterance_[item.text] = item.enqueueTime;
    return item;
}

void SpeechQueue::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    while (!queue_.empty()) {
        queue_.pop();
    }
    lastUtterance_.clear();
}

// ══════════════════════════════════════════════════════════════════════════════
// VoiceService
// ══════════════════════════════════════════════════════════════════════════════

VoiceService::VoiceService(VoiceSettings settings)
    : settings_(std::move(settings))
    , speechQueue_(settings.cooldownMs)
    , sapiVoice_(nullptr)
{
    engineHealth_.status = core::EngineStatus::Online;
    
    // Initialize SAPI
    HRESULT hr = CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL,
                                  IID_ISpVoice, &sapiVoice_);
    if (SUCCEEDED(hr) && sapiVoice_) {
        auto* voice = static_cast<ISpVoice*>(sapiVoice_);
        voice->SetRate(settings_.rate);
        voice->SetVolume(settings_.volume);
        
        // Start worker thread
        worker_ = std::thread(&VoiceService::workerThread, this);
    } else {
        engineHealth_.status = core::EngineStatus::Degraded;
        engineHealth_.detail = "Failed to initialize SAPI voice";
    }
}

VoiceService::~VoiceService() {
    running_.store(false, std::memory_order_release);
    speechQueue_.clear();
    
    if (worker_.joinable()) {
        worker_.join();
    }
    
    if (sapiVoice_) {
        auto* voice = static_cast<ISpVoice*>(sapiVoice_);
        voice->Release();
        sapiVoice_ = nullptr;
    }
}

void VoiceService::speak(const std::string& text, int priority) {
    if (!enabled_.load(std::memory_order_relaxed) || !sapiVoice_) {
        return;
    }
    
    speakImpl(text);
}

void VoiceService::speakAsync(const std::string& text, int priority) {
    if (!enabled_.load(std::memory_order_relaxed)) {
        return;
    }
    
    speechQueue_.enqueue(text, priority);
}

void VoiceService::setEnabled(bool enabled) {
    enabled_.store(enabled, std::memory_order_release);
    if (!enabled) {
        speechQueue_.clear();
    }
}

bool VoiceService::isEnabled() const noexcept {
    return enabled_.load(std::memory_order_relaxed);
}

void VoiceService::stop() {
    speechQueue_.clear();
    if (sapiVoice_) {
        auto* voice = static_cast<ISpVoice*>(sapiVoice_);
        voice->Speak(nullptr, SPF_PURGEBEFORESPEAK | SPF_ASYNC, nullptr);
    }
}

core::EngineHealth VoiceService::health() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return engineHealth_;
}

void VoiceService::setRate(int rate) {
    std::lock_guard<std::mutex> lock(mutex_);
    settings_.rate = rate;
    if (sapiVoice_) {
        auto* voice = static_cast<ISpVoice*>(sapiVoice_);
        voice->SetRate(rate);
    }
}

void VoiceService::setVolume(int volume) {
    std::lock_guard<std::mutex> lock(mutex_);
    settings_.volume = std::clamp(volume, 0, 100);
    if (sapiVoice_) {
        auto* voice = static_cast<ISpVoice*>(sapiVoice_);
        voice->SetVolume(settings_.volume);
    }
}

void VoiceService::setCooldownMs(int ms) {
    speechQueue_.setCooldownMs(ms);
    settings_.cooldownMs = ms;
}

void VoiceService::workerThread() {
    // Initialize COM for this thread
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    
    while (running_.load(std::memory_order_acquire)) {
        if (speechQueue_.hasPending()) {
            auto item = speechQueue_.dequeue();
            if (!item.text.empty()) {
                speakImpl(item.text);
            }
        }
        
        // Small sleep to prevent busy-wait
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    CoUninitialize();
}

void VoiceService::speakImpl(const std::string& text) {
    if (!sapiVoice_) return;
    
    auto* voice = static_cast<ISpVoice*>(sapiVoice_);
    
    // Convert to wide string
    const int wideLen = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (wideLen <= 0) return;
    
    std::wstring wideText(wideLen, 0);
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wideText[0], wideLen);
    
    // Speak
    HRESULT hr = voice->Speak(wideText.c_str(), SPF_ASYNC, nullptr);
    
    if (FAILED(hr)) {
        std::lock_guard<std::mutex> lock(mutex_);
        engineHealth_.status = core::EngineStatus::Degraded;
        engineHealth_.detail = "SAPI speak failed";
    }
}

}  // namespace csx::voice
