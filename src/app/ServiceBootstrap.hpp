#pragma once

#include "Application.hpp"

namespace csx {

// ──────────────────────────────────────────────────────────────────────────────
// ServiceBootstrap — helper for dependency injection and service wiring
// ──────────────────────────────────────────────────────────────────────────────
class ServiceBootstrap {
public:
    static void wireAlertManager(alerts::AlertManager* alertManager,
                                 voice::VoiceService* voiceService,
                                 recording::Recorder* recorder);
    
    static void wireThreatEngine(threat::ThreatEngine* threatEngine,
                                  behavior::BehaviorEngine* behaviorEngine,
                                  recognition::FacePipeline* facePipeline,
                                  tracking::ObjectTracker* tracker);
    
    static void wireRadar(radar::RadarModel* radarModel,
                          tracking::ObjectTracker* tracker,
                          threat::ThreatEngine* threatEngine);
};

}  // namespace csx
