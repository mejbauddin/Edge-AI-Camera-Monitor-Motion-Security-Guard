#pragma once

#include "AlertPolicy.hpp"
#include "interfaces/Interfaces.hpp"
#include "types/Frame.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

namespace csx::alerts {

// ──────────────────────────────────────────────────────────────────────────────
// AlertManager — coordinates alert generation, voice, and notifications
// ──────────────────────────────────────────────────────────────────────────────
class AlertManager : public core::IAlertManager {
public:
    explicit AlertManager(AlertPolicy policy = {});
    ~AlertManager() override = default;

    // IAlertManager
    void onThreatAssessment(const core::ThreatAssessment& assessment) override;
    void onFaceMatch(const core::FaceMatch& match) override;
    void onMotionDetected(const std::vector<core::Track>& tracks) override;
    void setVoiceService(core::IVoiceService* voice) override;
    void setRecorder(core::IRecorder* recorder) override;
    
    [[nodiscard]] std::vector<Alert> getActiveAlerts() const;
    void acknowledgeAlert(const std::string& alertId) override;
    void clearAcknowledged() override;
    
    [[nodiscard]] core::EngineHealth health() const override;

    // Configuration
    void setPolicy(const AlertPolicy& policy);
    [[nodiscard]] const AlertPolicy& policy() const noexcept { return policy_; }

private:
    void triggerAlert(const std::string& message, core::ThreatLevel level, std::uint32_t trackId = 0);
    bool shouldTrigger(core::ThreatLevel level) const;
    std::string generateVoiceMessage(core::ThreatLevel level, const std::string& context = "") const;

    mutable std::mutex mutex_;
    AlertPolicy policy_;
    std::vector<Alert> activeAlerts_;
    
    std::atomic<std::uint64_t> alertCount_{0};
    std::chrono::steady_clock::time_point lastAlertTime_;
    
    // Dependencies (injected via ServiceBootstrap in Phase 16)
    core::IVoiceService* voiceService_{nullptr};
    core::IRecorder* recorder_{nullptr};
    
    core::EngineHealth engineHealth_;
};

}  // namespace csx::alerts
