#include "ServiceBootstrap.hpp"

#include "alerts/AlertManager.hpp"
#include "voice/VoiceService.hpp"
#include "recording/Recorder.hpp"
#include "threat/ThreatEngine.hpp"
#include "behavior/BehaviorEngine.hpp"
#include "recognition/FacePipeline.hpp"
#include "tracking/ObjectTracker.hpp"
#include "radar/RadarModel.hpp"

namespace csx {

void ServiceBootstrap::wireAlertManager(alerts::AlertManager* alertManager,
                                         voice::VoiceService* voiceService,
                                         recording::Recorder* recorder)
{
    if (alertManager && voiceService) {
        alertManager->setVoiceService(voiceService);
    }
    if (alertManager && recorder) {
        alertManager->setRecorder(recorder);
    }
}

void ServiceBootstrap::wireThreatEngine(threat::ThreatEngine* threatEngine,
                                         behavior::BehaviorEngine* behaviorEngine,
                                         recognition::FacePipeline* facePipeline,
                                         tracking::ObjectTracker* tracker)
{
    // Wire threat engine dependencies
    // This would involve setting up the fusion engine with its inputs
    (void)threatEngine;
    (void)behaviorEngine;
    (void)facePipeline;
    (void)tracker;
}

void ServiceBootstrap::wireRadar(radar::RadarModel* radarModel,
                                   tracking::ObjectTracker* tracker,
                                   threat::ThreatEngine* threatEngine)
{
    // Wire radar model to receive track updates
    (void)radarModel;
    (void)tracker;
    (void)threatEngine;
}

}  // namespace csx
