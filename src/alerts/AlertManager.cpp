#include "AlertManager.hpp"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <random>

namespace csx::alerts {

AlertManager::AlertManager(AlertPolicy policy)
    : policy_(std::move(policy))
    , lastAlertTime_(std::chrono::steady_clock::now() - std::chrono::seconds(10))
{
    engineHealth_.status = core::EngineStatus::Online;
}

void AlertManager::onThreatAssessment(const core::ThreatAssessment& assessment) {
    if (!shouldTrigger(assessment.level)) {
        return;
    }
    
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastAlertTime_).count();
    
    if (elapsed < policy_.cooldownMs) {
        return;  // Cooldown period
    }
    
    std::string message = "Threat Level: " + toString(assessment.level);
    
    triggerAlert(message, assessment.level);
    lastAlertTime_ = now;
}

void AlertManager::onFaceMatch(const core::FaceMatch& match) {
    if (match.classification == core::IdentityClassification::Foe) {
        const std::string message = "Unauthorized individual detected";
        triggerAlert(message, core::ThreatLevel::Orange, match.trackId);
    }
}

void AlertManager::onMotionDetected(const std::vector<core::Track>& tracks) {
    // Check for unusual motion patterns (high velocity tracks)
    for (const auto& track : tracks) {
        if (!track.active) continue;
        
        const float speed = std::sqrt(track.velocity.x * track.velocity.x + 
                                       track.velocity.y * track.velocity.y);
        
        if (speed > 50.0F) {  // High speed threshold
            const std::string message = "High-velocity motion detected";
            triggerAlert(message, core::ThreatLevel::Yellow, track.id);
        }
    }
}

void AlertManager::setVoiceService(core::IVoiceService* voice) {
    voiceService_ = voice;
}

void AlertManager::setRecorder(core::IRecorder* recorder) {
    recorder_ = recorder;
}

std::vector<Alert> AlertManager::getActiveAlerts() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return activeAlerts_;
}

void AlertManager::acknowledgeAlert(const std::string& alertId) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& alert : activeAlerts_) {
        if (alert.id == alertId) {
            alert.acknowledged = true;
            break;
        }
    }
}

void AlertManager::clearAcknowledged() {
    std::lock_guard<std::mutex> lock(mutex_);
    activeAlerts_.erase(
        std::remove_if(activeAlerts_.begin(), activeAlerts_.end(),
                      [](const Alert& a) { return a.acknowledged; }),
        activeAlerts_.end());
}

core::EngineHealth AlertManager::health() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return engineHealth_;
}

void AlertManager::setPolicy(const AlertPolicy& policy) {
    std::lock_guard<std::mutex> lock(mutex_);
    policy_ = policy;
}

void AlertManager::triggerAlert(const std::string& message, core::ThreatLevel level, std::uint32_t trackId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Generate unique ID
    std::ostringstream oss;
    oss << "ALT-" << std::setfill('0') << std::setw(6) << alertCount_.fetch_add(1);
    
    Alert alert(message, level, trackId);
    alert.id = oss.str();
    activeAlerts_.push_back(std::move(alert));
    
    // Keep only recent alerts (max 100)
    if (activeAlerts_.size() > 100) {
        activeAlerts_.erase(activeAlerts_.begin());
    }
    
    // Trigger voice if enabled
    if (policy_.voiceEnabled && voiceService_) {
        const std::string voiceMsg = generateVoiceMessage(level, message);
        voiceService_->speakAsync(voiceMsg, static_cast<int>(level));
    }
    
    // Trigger recording if enabled and threat is high enough
    if (policy_.recordingTrigger && level >= core::ThreatLevel::Orange && recorder_) {
        recorder_->startRecording();
    }
}

bool AlertManager::shouldTrigger(core::ThreatLevel level) const {
    return level >= policy_.minTriggerLevel;
}

std::string AlertManager::generateVoiceMessage(core::ThreatLevel level, const std::string& context) const {
    switch (level) {
        case core::ThreatLevel::Green:
            return policy_.voiceGreen.empty() ? "System normal" : policy_.voiceGreen;
        case core::ThreatLevel::Yellow:
            return policy_.voiceYellow;
        case core::ThreatLevel::Orange:
            return policy_.voiceOrange;
        case core::ThreatLevel::Red:
            return policy_.voiceRed;
        case core::ThreatLevel::Critical:
            return policy_.voiceCritical;
    }
    return "Alert";
}

}  // namespace csx::alerts
